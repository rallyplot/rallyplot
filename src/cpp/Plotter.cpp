#include "include/Plotter.h"
#include "structure/SharedXData.h"
#include <qboxlayout.h>
#include <QLibrary>
#include <QApplication>
#include <csv.h>
#include <stdexcept>
#include <string>
#include <qapplication.h>
#include <qwidget.h>
#include <QPointer>
#include "structure/PlotWrapperWidget.h"


void checkStringIsValid(std::string str)
{
    for (char& c : str)
    {
        if (static_cast<unsigned char>(c) > 127)
        {
            throw std::runtime_error(
                "The character " + std::string(1, c) + "in string " + str + " is not valid. Only basic ASCII characters (0–127) are supported."
            );
        }
    }
}


struct SubKey
/*
    Used as std::unordered map index, to index active subplot.
 */
{
    int x;
    int y;

    explicit SubKey(int x_, int y_) : x(x_), y(y_) {}

    bool operator==(const SubKey& subplot) const
    {
        return (subplot.x == x && subplot.y == y);
    }
};


// Overload the hash function for <SubKey> to use
// it as as std::unordered_map key.
namespace std {
    template<>
    struct hash<SubKey> {
            std::size_t operator()(const SubKey& idx) const
            {
                return std::hash<int>()(idx.x) ^ (std::hash<int>()(idx.y) << 1);
            }
        };
};


class Plotter::Impl
/*

 */
{
public:

    Impl(PlotterArgs plotterArgs)
        : m_defaultConfigs(
              {plotterArgs.colorMode,
               plotterArgs.axisTickLabelFont,
               plotterArgs.axisTickLabelFontSize,
               plotterArgs.axisRight,
               plotterArgs.widthMarginSize,
               plotterArgs.heightMarginSize,
               plotterArgs.width,
               plotterArgs.height
          }
        )
    {

        m_passedPlotterArgs = plotterArgs;

        setupMainWidget();
    }

    void setupMainWidget()
    {
        // Set global OpenGL Options, must show here to setup the opengl context
        QSurfaceFormat openGLFormat;
        openGLFormat.setDepthBufferSize(24);
        openGLFormat.setStencilBufferSize(8);
        openGLFormat.setVersion(3, 3);
        openGLFormat.setProfile(QSurfaceFormat::CoreProfile);
        openGLFormat.setSwapInterval(1);
        openGLFormat.setSamples(m_passedPlotterArgs.antiAliasingSamples);
        QSurfaceFormat::setDefaultFormat(openGLFormat);

        // Setup (but do not exec) the window
        m_mainWidget = new QWidget();
        m_mainWidget->setWindowTitle("My Plotter");
        m_mainWidget->resize(m_passedPlotterArgs.width, m_passedPlotterArgs.height);

        m_centralLayout = new QGridLayout (m_mainWidget);

        m_mainwindowSubplots[SubKey{0, 0}] = new PlotWrapperWidget(m_mainWidget, m_defaultConfigs);

        m_centralLayout->addWidget(m_mainwindowSubplots[SubKey{0, 0}]);


        // Run show() so that OpenGL context etc. is initialised, but
        // don't actually show the widget yet until `start()` is called.
        // set `WA_DontShowOnScreen` because hide() doesn't work, the widget
        // must be shown so new subplots openGLWidget is initialsied properly.
        m_mainWidget->setAttribute(Qt::WA_DontShowOnScreen, true);
        m_mainWidget->show();

        // Set background for light or dark mode.
        QString stylesheet = utils_glmBackgroundColorToQtStylesheet(m_defaultConfigs.m_backgroundColor);
        m_mainWidget->setStyleSheet(stylesheet);
    }


    ~Impl()
    {
        // Must delete (not deleteLater()) or openGl
        // context is not torn down properly.
        m_mainwindowSubplots.clear();
        delete m_mainWidget;

        m_application.quit();

    }

    void start()
    {
        // We must do some tear-down after exec() so that when the window is closed
        // in interactive python, all existing plots are cleaned up and the widgets
        // refreshed. This means the plotter object can continue to be used without seg
        // fault risk.

        // Hide then show the widget (necessary to reset WA_DontShowOnScreen) before exec.
        m_mainWidget->hide();
        m_mainWidget->setAttribute(Qt::WA_DontShowOnScreen, false);
        m_mainWidget->show();

        m_application.exec();

        m_mainwindowSubplots.clear();

        delete m_mainWidget;

        setupMainWidget();
    }

    PlotWrapperWidget* activeSubplot()
    {
        m_mainwindowSubplots[SubKey{m_activeRow, m_activeCol}]->m_centralOpenGlWidget->makeCurrent();
        return m_mainwindowSubplots[SubKey{m_activeRow, m_activeCol}];
    }

    std::tuple<std::vector<std::uint8_t>, int, int> grabFrameBuffer(
        std::optional<int> row = std::nullopt, std::optional<int> col = std::nullopt
    )
    {
        if (!(row.has_value() == col.has_value()))
        {
            throw std::runtime_error("`row` and `col` must both be passed if one is.");
        }
        if (!row.has_value())
        {
            row = m_activeRow;
            col = m_activeCol;
        }
        checkActiveSubplotExists(row.value(), col.value());

        QImage rgba  = m_mainwindowSubplots[SubKey{row.value(), col.value()}]->m_centralOpenGlWidget->grabFramebuffer();

        std::vector<uint8_t> tight(rgba.width() * rgba.height() * 4);

        for (int y = 0; y < rgba.height(); ++y) {
            const uint8_t* src = rgba.constScanLine(y);
            uint8_t* dst = tight.data() + y * rgba.width() * 4;
            std::memcpy(dst, src, rgba.width() * 4);
        }
        this->m_mainWidget->show();
        return {tight, rgba .size().width(), rgba .size().height()};
    }

    std::string formatToString(QImage::Format format)
    {
        switch (format) {
        case QImage::Format_RGBA8888: return "Format_RGBA8888";
        case QImage::Format_ARGB32: return "Format_ARGB32";
        case QImage::Format_ARGB32_Premultiplied: return "Format_ARGB32_Premultiplied";
        default:
            return "Unknown format.";
        }
    }

    void resize(int width, int height)
    {
        m_mainWidget->resize(width, height);
    }

    /* -----------------------------------------------------------------------------------
     *  Settings
     * -----------------------------------------------------------------------------------
       Typically settings are specific to a liniked subplot (i.e. subplots that share an
       X-axis (within an high-level subplot). If the linked subplot index is passed, then
       the settings are only set for that linked subplot. Otherwise, set for all.
    */

    void setBackgroundColor(const float* ptr, int size)
    {
        activeSubplot()->configs().m_backgroundColor = utils_colorToGlmVec(ptr, size);
        activeSubplot()->setBackgroundColor(activeSubplot()->configs().m_backgroundColor);
    }

    void setCameraSettings(CameraSettings cameraSettings, std::optional<int> linkedSubplotIdx = std::nullopt)
    {
        if (linkedSubplotIdx.has_value())
        {
            activeSubplot()->linkedSubplot(linkedSubplotIdx.value())->setCameraSettings(
                cameraSettings
            );
        }
        else
        {
            for (const std::unique_ptr<LinkedSubplot>& subplot : activeSubplot()->allLinkedSubplots())
            {
                subplot->setCameraSettings(cameraSettings);
            }
        }
    }

    void setCrosshairSettings(CrosshairSettings crosshairSettings)
    /*
        Set at the opengl widget level as crosshair is Qt painted on
     */
    {
        throwExpectionOnInvalidColorOptional(crosshairSettings.lineColor);
        throwExpectionOnInvalidColorOptional(crosshairSettings.backgroundColor);
        throwExpectionOnInvalidColorOptional(crosshairSettings.fontColor);

        activeSubplot()->openGlWidget()->setCrosshairSettings(crosshairSettings);
    }

    void setHoverValueSettings(HoverValueSettings hoverValueSettings)
    /*
        Set at the opengl widget level as hover value is Qt painted on
     */
    {
        throwExpectionOnInvalidColorOptional(hoverValueSettings.fontColor);
        throwExpectionOnInvalidColorOptional(hoverValueSettings.backgroundColor);
        throwExpectionOnInvalidColorOptional(hoverValueSettings.borderColor);

        activeSubplot()->openGlWidget()->setHoverValueSettings(hoverValueSettings);
    }

    void setDrawLineSettings(DrawLineSettings drawLineSettings)
    {
        throwExpectionOnInvalidColorOptional(drawLineSettings.color);

        activeSubplot()->openGlWidget()->setDrawLineSettings(drawLineSettings);
    }

    void setXAxisSettings(AxisSettings xAxisSettings, std::optional<int> linkedSubplotIdx)
    {

        checkAxisSettings(xAxisSettings);

        if (linkedSubplotIdx.has_value())
        {
            activeSubplot()->linkedSubplot(linkedSubplotIdx.value())->setXAxisSettings(
                activeSubplot()->configs().convertToBackendAxisSettings(xAxisSettings)
            );
        }
        else
        {
            for (const std::unique_ptr<LinkedSubplot>& subplot : activeSubplot()->allLinkedSubplots())
            {
                subplot->setXAxisSettings(
                    activeSubplot()->configs().convertToBackendAxisSettings(xAxisSettings)
                );
            };
        }
    }

    void setYAxisSettings(AxisSettings yAxisSettings, std::optional<int> linkedSubplotIdx)
    {
        checkAxisSettings(yAxisSettings);

        if (linkedSubplotIdx.has_value())
        {
            activeSubplot()->linkedSubplot(linkedSubplotIdx.value())->setYAxisSettings(
                activeSubplot()->configs().convertToBackendAxisSettings(yAxisSettings)
            );
        }
        else
        {
            for (const std::unique_ptr<LinkedSubplot>& subplot : activeSubplot()->allLinkedSubplots())
            {
                subplot->setYAxisSettings(
                    activeSubplot()->configs().convertToBackendAxisSettings(yAxisSettings)
                );
            };
        }
    }

    void checkAxisSettings(AxisSettings axisSettings)
    {
        if (axisSettings.minNumTicks < 1)
        {
            throw std::invalid_argument("minNumTicks must be bigger than 0.");
        }

        if (axisSettings.maxNumTicks > cfg_MAX_NUM_TICKS)
        {
            throw std::invalid_argument("maxNumTicks cannot be bigger than " + std::to_string(cfg_MAX_NUM_TICKS) + ".");
        }

        throwExpectionOnInvalidColorOptional(axisSettings.gridlineColor);
        throwExpectionOnInvalidColorOptional(axisSettings.axisColor);
        throwExpectionOnInvalidColorOptional(axisSettings.fontColor);
    }

    /* -----------------------------------------------------------------------------------
     *  Set Axis Limits
     * -----------------------------------------------------------------------------------
       Similar to 'Settings', each linked subplot as its own axis limit settings.
    */

    void pinYAxis(bool on, std::optional<int> linkedSubplotIdx)
    /*
        Pins the y-axis limits to the data.
     */
    {
        if (linkedSubplotIdx.has_value())
        {
            activeSubplot()->linkedSubplot(linkedSubplotIdx.value())->pinYAxis(
                on
            );
        }
        else
        {
            for (const std::unique_ptr<LinkedSubplot>& subplot : activeSubplot()->allLinkedSubplots())
            {
                subplot->pinYAxis(on);
            }
        }
    }

    void setXLimits(
        std::optional<std::variant<int, std::string, std::chrono::system_clock::time_point>> min,
        std::optional<std::variant<int, std::string, std::chrono::system_clock::time_point>> max
    )
    /*
        Set the x-axis limits. The limits can be passed as a int or string (x-axis label). If the limit is not
        set, the edge of the data is used. The inputs are converted to int and used as x-axis limits.

        TODO: this class is to high-level to be performing such low-level operations.
    */
    {
        double passedMin;
        double passedMax;

        double delta = activeSubplot()->linkedSubplot(0)->jointPlotData().getDelta();

        if (min.has_value())
        {
            if (std::holds_alternative<int>(min.value()))
            {
                passedMin = (double)std::get<int>(min.value()) * delta;
            }
            else if (std::holds_alternative<std::chrono::system_clock::time_point>(min.value()))
            {
                std::vector<std::chrono::system_clock::time_point> datetimeVector = { std::get<std::chrono::system_clock::time_point>(min.value()) };
                passedMin = (double)activeSubplot()->sharedXData().convertDateToIndex(datetimeVector)[0] * delta;
            }
            else
            {
                std::vector<std::string> stringVector = {std::get<std::string>(min.value())};
                passedMin = (double)activeSubplot()->sharedXData().convertDateToIndex(stringVector)[0] * delta;
            }
        }
        else
        {
            passedMin = activeSubplot()->linkedSubplot(0)->jointPlotData().getDataMinX();
        }

        if (max.has_value())
        {
            if (std::holds_alternative<int>(max.value()))
            {
                passedMax = (double)std::get<int>(max.value()) * delta;
            }
            else if (std::holds_alternative<std::chrono::system_clock::time_point>(max.value()))
            {
                std::vector<std::chrono::system_clock::time_point> datetimeVector = { std::get<std::chrono::system_clock::time_point>(max.value()) };
                passedMax = (double)activeSubplot()->sharedXData().convertDateToIndex(datetimeVector)[0] * delta;
            }
            else
            {
                std::vector<std::string> stringVector = {std::get<std::string>(max.value())};
                passedMax = (double)activeSubplot()->sharedXData().convertDateToIndex(stringVector)[0] * delta;
            }
        }
        else
        {
            passedMax = activeSubplot()->linkedSubplot(0)->jointPlotData().getDataMaxX();
        }

        for (const std::unique_ptr<LinkedSubplot>& subplot : activeSubplot()->allLinkedSubplots())
        {
            subplot->camera().m_left = passedMin;
            subplot->camera().m_right = passedMax;
        }

        // TODO: This is a bit weird to change these configs directly from this high level.
        // The re-init xTicks is a hack (?) to update the first ticks to respec the new limits
        // For sure none of this should be done here.
        activeSubplot()->configs().m_xAxisLimitsOn = true;
        activeSubplot()->configs().m_xAxisLimits = {passedMin, passedMax};
        for (const std::unique_ptr<LinkedSubplot>& subplot : activeSubplot()->allLinkedSubplots())
        {
            subplot->axesObject().initXTicks(subplot->jointPlotData().getNumDatapoints());
        }
    }

    void setYLimits(std::optional<float> min, std::optional<float> max, std::optional<int> linkedSubplotIdx)
    {
        if (linkedSubplotIdx.has_value())
        {
            activeSubplot()->linkedSubplot(linkedSubplotIdx.value())->setYLimits(
                min, max
            );
        }
        else
        {
            for (const std::unique_ptr<LinkedSubplot>& subplot : activeSubplot()->allLinkedSubplots())
            {
                subplot->setYLimits(min, max);
            }
        }

        // Refresh the y ticks to respect the new limits. This shouldnt really be done here, see setXLimits
        for (const std::unique_ptr<LinkedSubplot>& subplot : activeSubplot()->allLinkedSubplots())
        {
            subplot->axesObject().initYTicks();
        }
    }

    void linkYAxis(bool on)
    /*
        When the plot is zoomed or panned, the y-axis of all linked subplots is moved together.
        In most instances, this is not needed because the y-axis will have different units / scales.
     */
    {
        activeSubplot()->configs().m_linkYZoomAndPan = on;
        this->pinYAxis(false, std::nullopt);
    }

    /* -----------------------------------------------------------------------------------
     *  Labels and legend
     * ----------------------------------------------------------------------------------- */

    void setXLabel(std::string text, std::optional<AxisLabelSettings> settings)
    {
        if (settings.has_value())
        {
            throwExpectionOnInvalidColorOptional(settings.value().color);
        }

        AxisLabelSettings passedSettings{
            settings.value_or(AxisLabelSettings{})
        };

        activeSubplot()->setXLabel(text, passedSettings);
    }

    void setYLabel(std::string text, std::optional<AxisLabelSettings> settings)
    {
        if (settings.has_value())
        {
            throwExpectionOnInvalidColorOptional(settings.value().color);
        }

        AxisLabelSettings passedSettings{
            settings.value_or(AxisLabelSettings{})
        };

        activeSubplot()->setYLabel(text, passedSettings);
    }

    void setTitle(std::string text, std::optional<TitleLabelSettings> settings)
    {
        if (settings.has_value())
        {
            throwExpectionOnInvalidColorOptional(settings.value().color);
        }

        TitleLabelSettings passedSettings{
            settings.value_or(TitleLabelSettings{})
        };

        activeSubplot()->setTitle(text, passedSettings);
    }

    void setLegend(
        std::variant<std::vector<std::string>, std::vector<LegendItem>> labels,
        int linkedSubplotIdx,
        LegendSettings legendSettings
        )
    {
        std::size_t labelSize = std::visit([](const auto& vec) { return vec.size(); }, labels);

        int numPlots = activeSubplot()->linkedSubplot(linkedSubplotIdx)->jointPlotData().plotVector().size();

        if (labelSize != numPlots)
        {
            throw std::invalid_argument(
                "The number of labels in the legend being set must equal the number of plots. \n"
                "There are " + std::to_string(labelSize) + " labels, but " + std::to_string(numPlots) + " plots."
            );
        }

        int numLinkedSubplots = activeSubplot()->openGlWidget()->m_rm->m_linkedSubplots.size();

        if (linkedSubplotIdx >= numLinkedSubplots)
        {
            throw std::invalid_argument(
                "The `linkedSubplotIdx`: " + std::to_string(linkedSubplotIdx) +
                " is larger than the number of plots: " + std::to_string(numLinkedSubplots)
            );
        }

        if (std::holds_alternative<std::vector<LegendItem>>(labels))
        {
            const std::vector<LegendItem>& labelsVector = std::get<std::vector<LegendItem>>(labels);

            for (const LegendItem& item : labelsVector)
            {
                throwExpectionOnInvalidColor(item.leftColor);
                throwExpectionOnInvalidColor(item.rightColor);
            }
        }
        throwExpectionOnInvalidColorOptional(legendSettings.fontColor);
        throwExpectionOnInvalidColor(legendSettings.boxColor);

        int linkedIdx = activeSubplot()->accountForNegativeIdx(linkedSubplotIdx);

        activeSubplot()->openGlWidget()->setLegend(labels, linkedIdx, legendSettings);
    }

    /* -----------------------------------------------------------------------------------
     *  Subplots
     * ----------------------------------------------------------------------------------- */

    void addSubplot(int row, int col, int rowSpan, int colSpan)
    /*
        Add a subplot to the mainwindow and insert it in the unordered map.
     */
    {
        auto it = m_mainwindowSubplots.find(SubKey{row, col});

        if (it == m_mainwindowSubplots.end())
        {
            m_mainwindowSubplots[SubKey{row, col}] = new PlotWrapperWidget(m_mainWidget, m_defaultConfigs);

            m_centralLayout->addWidget(m_mainwindowSubplots[SubKey{row, col}], row, col, rowSpan, colSpan);

            setActiveSubplot(row, col);
        }
        else
        {
            m_centralLayout->removeWidget(m_mainwindowSubplots[SubKey{row, col}]);
            m_centralLayout->addWidget(m_mainwindowSubplots[SubKey{row, col}], row, col, rowSpan, colSpan);

            setActiveSubplot(row, col);
        }
    }

    void setActiveSubplot(int row, int col)
    {
        checkActiveSubplotExists(row, col);

        m_activeRow = row;
        m_activeCol = col;
    }


    void checkActiveSubplotExists(int row, int col)
    {
        if (m_mainwindowSubplots.find(SubKey{row, col}) == m_mainwindowSubplots.end())
        {
            throw std::runtime_error(
                "Cannot activate subplot at row: " + std::to_string(row) + ", col: "
                + std::to_string(col) + ", it does not exist."
                );
        }
    }

    void addLinkedSubplot(double heightAsProportion)
    {
        activeSubplot()->renderManager()->addLinkedSubplot(heightAsProportion);
    }

    void resizeLinkedSubplots(std::vector<double> yHeights)
    {
        if (yHeights.size() != activeSubplot()->renderManager()->m_linkedSubplots.size())
        {
            throw std::runtime_error("Number of subplot heights must match the number of subplots");
        }
        activeSubplot()->renderManager()->resizeLinkedSubplots(yHeights);
    }

    /* -----------------------------------------------------------------------------------
     *  Plots
     * ----------------------------------------------------------------------------------- */

    void candlestick(
        const float* openPtr, std::size_t openSize,
        const float* highPtr, std::size_t highSize,
        const float* lowPtr, std::size_t lowSize,
        const float* closePtr, std::size_t closeSize,
        OptionalDateVector dates,
        std::optional<CandlestickSettings> candlestickSettings,
        int linkedSubplotIdx
    )
    {
        if (!(openSize == closeSize && openSize == lowSize && openSize == highSize))
        {
            throw std::invalid_argument("Candlestick open, high, low, close vectors are not all the same size.");
        }

        throwExpectionForFailedPlotChecks(openSize, dates, linkedSubplotIdx);

        if (candlestickSettings.has_value())
        {
            throwExpectionOnInvalidColor(candlestickSettings.value().upColor);
            throwExpectionOnInvalidColor(candlestickSettings.value().downColor);
        }

        BackendCandlestickSettings backendSettings{
            candlestickSettings.value_or(CandlestickSettings{})
        };

        activeSubplot()->linkedSubplot(linkedSubplotIdx)->candlestick(
            openPtr, openSize,
            highPtr, highSize,
            lowPtr, lowSize,
            closePtr, closeSize,
            dates,
            backendSettings
        );
    }

    void line(
        const float* yPtr,
        std::size_t ySize,
        OptionalDateVector dates,
        std::optional<LineSettings> lineSettings,
        int linkedSubplotIdx
    )
    {
        throwExpectionForFailedPlotChecks(ySize, dates, linkedSubplotIdx);

        BackendLineSettings backendSettings{
            lineSettings.value_or(LineSettings{})
        };

        activeSubplot()->linkedSubplot(linkedSubplotIdx)->line(yPtr, ySize, dates, backendSettings);
    }

    void bar(
        const float* yPtr,
        std::size_t ySize,
        OptionalDateVector dates,
        std::optional<BarSettings> barSettings,
        int linkedSubplotIdx
    )
    {
        throwExpectionForFailedPlotChecks(ySize, dates, linkedSubplotIdx);

        if (barSettings.has_value())
        {
            throwExpectionOnInvalidColor(barSettings.value().color);
        }

        BackendBarSettings backendSettings{
            barSettings.value_or(BarSettings{})
        };

        activeSubplot()->linkedSubplot(linkedSubplotIdx)->bar(yPtr, ySize, dates, backendSettings);
    }

    void scatter(
        ScatterDateVector xData, const float* yPtr, std::size_t ySize, std::optional<ScatterSettings> scatterSettings, int linkedSubplotIdx
    )
    {
        BackendScatterSettings backendSettings{
            scatterSettings.value_or(ScatterSettings{})
        };

        if (activeSubplot()->linkedSubplot(linkedSubplotIdx)->jointPlotData().isEmpty())
        {
            throw std::runtime_error("`scatter` cannot be the first plot. It must be overlaid onto another plot.");
        }

        DateType dateType = activeSubplot()->linkedSubplot(linkedSubplotIdx)->sharedXData().getDateType();

        if (std::holds_alternative<StringVectorRef>(xData))
        {
            if (dateType == DateType::Timepoint)
            {
                throw std::runtime_error("The x-axis data must always be string, index or chrono timepoint. Currently it is timepoint, but string data was passed.");
            }
            else if (dateType == DateType::NoDate)
            {
                throw std::runtime_error("The x-axis data must always be string, index or chrono timepoint. Currently it is an index, but string data was passed.");
            }
        }
        else if (std::holds_alternative<TimepointVectorRef>(xData))
        {
            if (dateType == DateType::String)
            {
                throw std::runtime_error("The x-axis data must always be string, index or chrono timepoint. Currently it is string, but timepoint data was passed.");
            }
            else if (dateType == DateType::NoDate)
            {
                throw std::runtime_error("The x-axis data must always be string, index or chrono timepoint. Currently it is an index, but timepoint data was passed.");
            }
        }

        activeSubplot()->linkedSubplot(linkedSubplotIdx)->scatter(xData, yPtr, ySize, backendSettings);
    }

    /* -----------------------------------------------------------------------------------
     * Input argument checks
     * ----------------------------------------------------------------------------------- */


    void throwExpectionOnInvalidColorOptional (std::optional<std::vector<float>> color)
    {
        if (color.has_value())
        {
            throwExpectionOnInvalidColor(color.value());
        }
    };


    void throwExpectionOnInvalidColor(std::vector<float> color)
    {
        if (color.size() == 0 || color.size() > 4)
        {
            throw std::invalid_argument("Color must be an array of length 1-4 (RGBA)");
        }

        for (int i = 0; i < color.size(); i++)
        {
            if (color[i] < 0.0f || color[i] > 1.0f)
            {
                throw std::invalid_argument("Color RGBA entries must be a value between 0 and 1.");
            }
        }
    }

    int getDatesSize(OptionalDateVector dates)
    {
        if (!dates.has_value())
        {
            return -1;
        }
        else if (std::holds_alternative<StringVectorRef>(dates.value()))
        {
            return std::get<StringVectorRef>(dates.value()).get().size();
        }
        else if (std::holds_alternative<TimepointVectorRef>(dates.value()))
        {
            return std::get<TimepointVectorRef>(dates.value()).get().size();
        }
        else
        {
            throw std::invalid_argument("Dates type not recognised.");
        }
    }

    void throwExpectionForFailedPlotChecks(
        std::size_t ySize,
        OptionalDateVector dates,
        int linkedSubplotIdx
    )
    {
        int numLinkedSubplots = activeSubplot()->allLinkedSubplots().size();

        if (linkedSubplotIdx >= numLinkedSubplots)
        {
            throw std::invalid_argument(
                "linkedSubplotIdx: " + std::to_string(linkedSubplotIdx) +
                "is larger than the number of plots: " + std::to_string(numLinkedSubplots)
            );
        }

        if (dates.has_value())
        {
            int datesSize = getDatesSize(dates);
            if (ySize != datesSize)
            {
                throw std::invalid_argument("Size of dates: " + std::to_string(datesSize) + " is different from the data size: " + std::to_string(ySize));
            }
        }

        if (!activeSubplot()->linkedSubplot(linkedSubplotIdx)->jointPlotData().isEmpty())
        {
            int numDataPoints = activeSubplot()->linkedSubplot(linkedSubplotIdx)->jointPlotData().getNumDatapoints();

            if (ySize != numDataPoints)
            {
                throw std::invalid_argument(
                    "ySize: " + std::to_string(ySize) + " does not match the number of datapoints on the plot "
                    + std::to_string(numDataPoints) + ". All plots must have the same number of data points."
                    );
            }

            if (dates.has_value())
            {
                if (numDataPoints != getDatesSize(dates))
                {
                 throw std::invalid_argument("dates vector length does not match the number of datapoints on the plot.");
                }
            }
        }

        // Check date types
        DateType dateType = activeSubplot()->linkedSubplot(linkedSubplotIdx)->sharedXData().getDateType();

        if (dates.has_value())
        {
            if (std::holds_alternative<StringVectorRef>(dates.value()))
            {
                if (dateType == DateType::Timepoint)
                {
                    throw std::invalid_argument("Dates are already set with string labels. Cannot now set a timepoint vector as dates.");
                }
                else if (dateType == DateType::String)
                {
                    throw std::runtime_error("Warning: string dates already exist on the plot. Dates will be updated.");
                }
            }
            else if (std::holds_alternative<TimepointVectorRef>(dates.value()))
            {
                if (dateType == DateType::String)
                {
                    throw std::invalid_argument("Dates are already set with timepoint labels. Cannot now set a string vector as dates.");
                }
                else if (dateType == DateType::Timepoint)
                {
                    throw std::runtime_error("Warning: timepoint dates already exist on the plot. Dates will be updated.");
                }
            }

        }
    }

    /* -----------------------------------------------------------------------------------
     *  Read from disk
     * ----------------------------------------------------------------------------------- */

    CandleDataCSV _readDataFromCSV(const std::string& dataFilepath) const
    /*
    Read a .csv with columns (open, close, low, high) and optionally
    (dates) into the candleData structure.
 */
    {
        CandleDataCSV candleData;

        float open, close, low, high;
        std::string dates;

        try {
            // Attempt to read the header with the "Date" column
            io::CSVReader<5> in(dataFilepath);
            in.read_header(io::ignore_extra_column, "Open", "Close", "Low", "High", "Date");

            while (in.read_row(open, close, low, high, dates)) {
                candleData.open.push_back(open);
                candleData.close.push_back(close);
                candleData.low.push_back(low);
                candleData.high.push_back(high);
                candleData.dates.push_back(dates);
            }

        } catch (const std::exception& e) {
            // If "Date" column is missing, fall back to reading without it
            io::CSVReader<4> in(dataFilepath);
            in.read_header(io::ignore_extra_column, "Open", "Close", "Low", "High");

            while (in.read_row(open, close, low, high)) {
                candleData.open.push_back(open);
                candleData.close.push_back(close);
                candleData.low.push_back(low);
                candleData.high.push_back(high);
            }
        }
        return candleData;
    }

private:

    int m_argc = 0;
    std::vector<char*> m_argv{ const_cast<char*>("") };
    QApplication m_application{m_argc, m_argv.data()};

    Configs m_defaultConfigs;

    int m_activeRow = 0;
    int m_activeCol = 0;

    std::unordered_map<SubKey, PlotWrapperWidget*> m_mainwindowSubplots;

    QPointer<QWidget> m_mainWidget = nullptr;


    QGridLayout* m_centralLayout;  // owned by m_mainWidget

    PlotterArgs m_passedPlotterArgs;
};


/* ---------------------------------------------------------------------------------
  Plotter
 ----------------------------------------------------------------------------------- */

Plotter::Plotter(
    std::optional<PlotterArgs> plotterArgs
)
{
    if (plotterArgs.has_value())
    {
        pImpl = std::make_unique<Impl>(
            plotterArgs.value()
        );
    }
    else
    {
        PlotterArgs defaultPlotArgs{};

        pImpl = std::make_unique<Impl>(
            defaultPlotArgs
        );
    }
};


Plotter::~Plotter() = default;

void Plotter::start()
{
    pImpl->start();
};

void Plotter::setBackgroundColor(const std::vector<float> backgroundColor)
{
    pImpl->setBackgroundColor(backgroundColor.data(), backgroundColor.size());
}

void Plotter::setCameraSettings(CameraSettings cameraSettings, std::optional<int> linkedSubplotIdx)
{
    pImpl->setCameraSettings(cameraSettings, linkedSubplotIdx);
}

void Plotter::pinYAxis(bool on, std::optional<int> linkedSubplotIdx)
{
    pImpl->pinYAxis(on, linkedSubplotIdx);
}

void Plotter::linkYAxis(bool on)
{
    pImpl->linkYAxis(on);
}

void Plotter::setYLimits(std::optional<double> min, std::optional<double> max, std::optional<int> linkedSubplotIdx)
{
    pImpl->setYLimits(min, max, linkedSubplotIdx);
}

void Plotter::setXLimits(
    std::optional<std::variant<int, std::string, std::chrono::system_clock::time_point>> min,
    std::optional<std::variant<int, std::string, std::chrono::system_clock::time_point>> max)
{
    pImpl->setXLimits(min, max);
}

void Plotter::setXLabel(std::string text, std::optional<AxisLabelSettings> settings)
{
    pImpl->setXLabel(text, settings);
}

void Plotter::setYLabel(std::string text, std::optional<AxisLabelSettings> settings)
{
    pImpl->setYLabel(text, settings);
}

void Plotter::setTitle(std::string text, std::optional<TitleLabelSettings> settings)
{
    pImpl->setTitle(text, settings);
}

void Plotter::setXAxisSettings(XAxisSettings xAxisSettings, std::optional<int> linkedSubplotIdx)
{
    pImpl->setXAxisSettings(
        xAxisSettings, linkedSubplotIdx
    );
}

void Plotter::setYAxisSettings(YAxisSettings yAxisSettings, std::optional<int> linkedSubplotIdx)
{
    pImpl->setYAxisSettings(
        yAxisSettings, linkedSubplotIdx
    );
}


void Plotter::resize(int width, int height)
{
    pImpl->resize(width, height);
}


void Plotter::candlestick(
    const std::vector<float>& open,
    const std::vector<float>& high,
    const std::vector<float>& low,
    const std::vector<float>& close,
    const std::vector<std::string>& dates,
    std::optional<CandlestickSettings> candlestickSettings,
    int linkedSubplotIdx
)
{
    pImpl->candlestick(
        open.data(), open.size(),
        high.data(), high.size(),
        low.data(), low.size(),
        close.data(), close.size(),
        dates,
        candlestickSettings,
        linkedSubplotIdx
    );
}


void Plotter::candlestick(
    const std::vector<float>& open,
    const std::vector<float>& high,
    const std::vector<float>& low,
    const std::vector<float>& close,
    const std::vector<std::chrono::system_clock::time_point>& dates,
    std::optional<CandlestickSettings> candlestickSettings,
    int linkedSubplotIdx
)
{
    pImpl->candlestick(
        open.data(), open.size(),
        high.data(), high.size(),
        low.data(), low.size(),
        close.data(), close.size(),
        dates,
        candlestickSettings,
        linkedSubplotIdx
    );
}


void Plotter::candlestick(
    const std::vector<float>& open,
    const std::vector<float>& high,
    const std::vector<float>& low,
    const std::vector<float>& close,
    std::optional<CandlestickSettings> candlestickSettings,
    int linkedSubplotIdx
    )
{
    pImpl->candlestick(
        open.data(), open.size(),
        high.data(), high.size(),
        low.data(), low.size(),
        close.data(), close.size(),
        std::nullopt,
        candlestickSettings,
        linkedSubplotIdx
    );
}

void Plotter::candlestick(
    const float* openPtr, std::size_t openSize,
    const float* highPtr, std::size_t highSize,
    const float* lowPtr, std::size_t lowSize,
    const float* closePtr, std::size_t closeSize,
    const OptionalDateVector dates,
    std::optional<CandlestickSettings> candlestickSettings,
    int linkedSubplotIdx
)
{
    pImpl->candlestick(
        openPtr, openSize,
        highPtr, highSize,
        lowPtr, lowSize,
        closePtr, closeSize,
        dates,
        candlestickSettings,
        linkedSubplotIdx
    );
}

void Plotter::line(
    const std::vector<float>& yData,
    const OptionalDateVector dates,
    std::optional<LineSettings> lineSettings,
    int linkedSubplotIdx
)
{
    pImpl->line(yData.data(), yData.size(), dates, lineSettings, linkedSubplotIdx);
}

void Plotter::line(
    const float* yPtr, std::size_t ySize,
    const OptionalDateVector dates,
    std::optional<LineSettings> lineSettings,
    int linkedSubplotIdx
    )
{
    pImpl->line(yPtr, ySize, dates, lineSettings, linkedSubplotIdx);
}

void Plotter::line(
    const std::vector<float>& yData,
    std::optional<LineSettings> lineSettings,
    int linkedSubplotIdx
    )
{
    pImpl->line(yData.data(), yData.size(), std::nullopt, lineSettings, linkedSubplotIdx);
}

void Plotter::bar(
    const std::vector<float>& yData,
    const OptionalDateVector dates,
    std::optional<BarSettings> barSettings,
    int linkedSubplotIdx
)
{
    pImpl->bar(yData.data(), yData.size(), dates, barSettings, linkedSubplotIdx);
}

void Plotter::bar(
    const std::vector<float>& yData,
    std::optional<BarSettings> barSettings,
    int linkedSubplotIdx
)
{
    pImpl->bar(yData.data(), yData.size(), std::nullopt, barSettings, linkedSubplotIdx);
}

void Plotter::bar(
    const float* yPtr, std::size_t ySize,
    const OptionalDateVector dates,
    std::optional<BarSettings> barSettings,
    int linkedSubplotIdx
)
{
    pImpl->bar(yPtr, ySize, dates, barSettings, linkedSubplotIdx);
}


void Plotter::scatter(
    const std::vector<int>& xData,
    const std::vector<float>& yData,
    std::optional<ScatterSettings> scatterSettings,
    int linkedSubplotIdx
)
{
    StdPtrVector<int> xDataVector = StdPtrVector<int>(xData.data(), xData.size());  // passed by value down to ScatterPlotData where it is stored

    pImpl->scatter(xDataVector, yData.data(), yData.size(), scatterSettings, linkedSubplotIdx);
}


void Plotter::scatter(
    const std::vector<std::string>& xData,
    const float* yPtr, std::size_t ySize,
    std::optional<ScatterSettings> scatterSettings,
    int linkedSubplotIdx
)
{
    pImpl->scatter(xData, yPtr, ySize, scatterSettings, linkedSubplotIdx);
}


void Plotter::scatter(
    const std::vector<std::chrono::system_clock::time_point>& xData,
    const float* yPtr, std::size_t ySize,
    std::optional<ScatterSettings> scatterSettings,
    int linkedSubplotIdx
)
{
    pImpl->scatter(xData, yPtr, ySize, scatterSettings, linkedSubplotIdx);

}


void Plotter::scatter(
    const std::vector<std::string>& xData,
    const std::vector<float>& yData,
    std::optional<ScatterSettings> scatterSettings,
    int linkedSubplotIdx
)
{
    pImpl->scatter(xData, yData.data(), yData.size(), scatterSettings, linkedSubplotIdx);

}


void Plotter::scatter(
    const std::vector<std::chrono::system_clock::time_point>& xData,
    const std::vector<float>& yData,
    std::optional<ScatterSettings> scatterSettings,
    int linkedSubplotIdx
    )
{
    pImpl->scatter(xData, yData.data(), yData.size(), scatterSettings, linkedSubplotIdx);

}


void Plotter::scatter(
    const int* xPtr, std::size_t xSize,
    const float* yPtr, std::size_t ySize,
    std::optional<ScatterSettings> scatterSettings,
    int linkedSubplotIdx
    )
{
    StdPtrVector<int> xDataVector = StdPtrVector<int>(xPtr, xSize);  // passed by value down to ScatterPlotData where it is stored

    pImpl->scatter(xDataVector, yPtr, ySize, scatterSettings, linkedSubplotIdx);
}

void Plotter::addLinkedSubplot(
    double heightAsProportion
)
{
    pImpl->addLinkedSubplot(heightAsProportion);
}


void Plotter::resizeLinkedSubplots(std::vector<double> yHeights)
{
    pImpl->resizeLinkedSubplots(yHeights);
}


void Plotter::setActiveSubplot(int row, int col)
{
    pImpl->setActiveSubplot(row, col);
}


CandleDataCSV Plotter::_readDataFromCSV(const std::string& dataFilepath) const
{
    return pImpl->_readDataFromCSV(dataFilepath);
}


void Plotter::setLegend(
    std::vector<std::string> labelNames, std::optional<LegendSettings> legendSettings, int linkedSubplotIdx
)
{
    for (auto& name : labelNames)
    {
        checkStringIsValid(name);
    }

    LegendSettings usedSettings{
        legendSettings.value_or(LegendSettings{})
    };

    pImpl->setLegend(labelNames, linkedSubplotIdx, usedSettings);
}

void Plotter::setLegend(
    std::vector<LegendItem> labelItems, std::optional<LegendSettings> legendSettings, int linkedSubplotIdx
)
{
    for (auto& labelItem : labelItems)
    {
        checkStringIsValid(labelItem.name);
    }

    LegendSettings usedSettings{
        legendSettings.value_or(LegendSettings{})
    };

    pImpl->setLegend(labelItems, linkedSubplotIdx, usedSettings);
}


void Plotter::setCrosshairSettings(CrosshairSettings crosshairSettings)
{
    pImpl->setCrosshairSettings(crosshairSettings);
}


void Plotter::setDrawLineSettings(DrawLineSettings drawLineSettings)
{
    pImpl->setDrawLineSettings(drawLineSettings);
}


void Plotter::setHoverValueSettings(HoverValueSettings hoverValueSettings)
{
    pImpl->setHoverValueSettings(hoverValueSettings);
}


void Plotter::addSubplot(int row, int col, int rowSpan, int colSpan)
{
    pImpl->addSubplot(row, col, rowSpan, colSpan);
}


std::tuple<std::vector<uint8_t>, int, int> Plotter::_grabFrameBuffer(
    std::optional<int> row, std::optional<int> col
)
{
    return pImpl->grabFrameBuffer(row, col);
}
