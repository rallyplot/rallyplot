#include "LinkedSubplot.h"
#include "../charts/plots/ScatterPlot.h"
#include "RenderManager.h"
#include "../charts/plots/CandlestickPlot.h"
#include "../charts/plots/LinePlot.h"
#include "../charts/plots/BarPlot.h"
#include "../charts/plots/ScatterPlot.h"
#include "../charts/plots/BarPlot.h"
#include "../Configs.h"


LinkedSubplot::LinkedSubplot(
    RenderManager& renderManager,
    Configs& configs,
    BackendCameraSettings subplotCameraSettings,
    BackendAxisSettings subplotXAxisSettings,
    BackendAxisSettings subplotYAxisSettings,
    SharedXData& sharedXData,
    QOpenGLFunctions_3_3_Core& glFunctions,
    WindowViewportObject& windowViewport,
    double yStartProportion,
    double yHeightProportion
)
  : m_yStartProportion(yStartProportion),
    m_yHeightProportion(yHeightProportion),
    m_rm(renderManager),
    m_configs(configs),
    m_linkedSubplotCameraSettings(subplotCameraSettings),
    m_linkedSubplotXAxisSettings(subplotXAxisSettings),
    m_linkedSubplotYAxisSettings(subplotYAxisSettings),
    m_sharedXData(sharedXData),
    m_gl(glFunctions),
    m_windowViewport(windowViewport),
    m_JointPlotData(*this),
    m_camera(configs, *this, windowViewport, m_JointPlotData),
    m_axisTickLabels(m_configs, *this, glFunctions),  // TODO: read everything off subplot??? makes most sense I think.... TODO TODO TODO
    m_axesObject(configs, sharedXData, *this, glFunctions)
{
    if (m_yHeightProportion <= 0.50){
        m_linkedSubplotYAxisSettings.minNumTicks = 4;
        m_linkedSubplotYAxisSettings.maxNumTicks = 8;
    }
    if (m_yHeightProportion <= 0.25){
        m_linkedSubplotYAxisSettings.maxNumTicks = 3;
        m_linkedSubplotYAxisSettings.maxNumTicks = 6;
    }
}


void LinkedSubplot::draw()
{

    glm::mat4 NDCMatrix = m_camera.getNDCMatrix();


    // Draw the Yaxis
    // -----------------------------------------------------------------

    glm::mat4 viewportTransform = m_windowViewport.setForLinkedSubplotYAxis(m_yStartProportion, m_yHeightProportion);
    m_axesObject.drawAxes(viewportTransform, "y");
    m_axesObject.drawYAxesAndTicks(viewportTransform, NDCMatrix, m_yHeightProportion);

    // Draw the Plots within the restricted viewport (inside the axes)
    // (first, so they are in front of gridlines)
    // -----------------------------------------------------------------

    m_windowViewport.setForLinkedSubplotPlot(m_yStartProportion, m_yHeightProportion);

    m_JointPlotData.draw(NDCMatrix, m_camera);

    if (m_drawLines.size() > 0)
    {
        for (const std::unique_ptr<DrawLine>& linePlot : m_drawLines)
        {
            linePlot->draw(NDCMatrix, m_camera);
        }
    }

    if (m_legend)
    {
        m_legend->draw(m_camera);
    }
}


void LinkedSubplot::setupFirstPlot(int numElements)
{
    m_camera.setupView();
    m_axesObject.initXTicks(numElements);
    m_axesObject.initYTicks();
}


/* Candlestick Plot
--------------------------------------------------------------------- */

// TODO: add option here for labels!
// TODO: need to be very careful about std::vector<float> moves! using the pointer will be much
// better after all actually...

void LinkedSubplot::candlestick(
    const float* openPtr, std::size_t openSize,
    const float* highPtr, std::size_t highSize,
    const float* lowPtr, std::size_t lowSize,
    const float* closePtr, std::size_t closeSize,
    OptionalDateVector dates,
    BackendCandlestickSettings backendSettings
)
{
    if (dates.has_value())
    {
        m_sharedXData.handleNewXDataVector(dates.value());
    }

    std::unique_ptr<CandlestickPlot> candlestick = std::make_unique<CandlestickPlot>(
        m_configs,
        *this,
        backendSettings,
        m_gl,
        openPtr, openSize,
        highPtr, highSize,
        lowPtr, lowSize,
        closePtr, closeSize
    );

    m_JointPlotData.addPlot(
        std::move(candlestick)
    );

    // TODO: OWN FUNC?
    if (m_JointPlotData.numPlots() == 1)
    {
        setupFirstPlot(m_JointPlotData.getNumDatapoints());
    }

    m_camera.setYLimitsFromView();
    if (m_linkedSubplotCameraSettings.yAxisLimitMode == YAxisMode::FixedAuto)
    {
        updateYAxisLimits();
    }

}


/* Scatter Plot
--------------------------------------------------------------------- */

void LinkedSubplot::scatter(
    ScatterDateVector xData,
    const float* yPtr,
    std::size_t ySize,
    BackendScatterSettings backendSettings
)
{
    std::unique_ptr<ScatterPlot> scatter = std::make_unique<ScatterPlot>(
        backendSettings, m_sharedXData, m_gl, xData, yPtr, ySize, *this
    );

    m_JointPlotData.addPlot(
        std::move(scatter)
    );

    m_camera.setYLimitsFromView();
    if (m_linkedSubplotCameraSettings.yAxisLimitMode == YAxisMode::FixedAuto)
    {
        updateYAxisLimits();
    }
}


/* Line Plot
--------------------------------------------------------------------- */

void LinkedSubplot::line(
    const float* yPtr, std::size_t ySize,
    OptionalDateVector dates,
    BackendLineSettings backendSettings
)
{
    if (dates.has_value())
    {
        m_sharedXData.handleNewXDataVector(dates.value());
    }

    std::unique_ptr<LinePlot> linePlot = std::make_unique<LinePlot>(
        m_configs,
        backendSettings,
        *this,
        m_gl,
        yPtr, ySize
    );

    m_JointPlotData.addPlot(
        std::move(linePlot)
    );

    // TODO: OWN FUNC
    if (m_JointPlotData.numPlots() == 1)
    {
        setupFirstPlot(m_JointPlotData.getNumDatapoints());
    }
    m_camera.setYLimitsFromView();
    if (m_linkedSubplotCameraSettings.yAxisLimitMode == YAxisMode::FixedAuto)
    {
        updateYAxisLimits();
    }
}


/* Bar Plot
--------------------------------------------------------------------- */


void LinkedSubplot::bar(
    const float* yPtr, std::size_t ySize,
    OptionalDateVector date,
    BackendBarSettings backendSettings
)
{
    if (date.has_value())
    {
        m_sharedXData.handleNewXDataVector(date.value());
    }

    std::unique_ptr<BarPlot> bar = std::make_unique<BarPlot>(
        m_configs,
        backendSettings,
        m_gl,
        yPtr, ySize
    );

    m_JointPlotData.addPlot(
        std::move(bar)
    );

    // TODO: OWN FUNC
    if (m_JointPlotData.numPlots() == 1)
    {
        setupFirstPlot(m_JointPlotData.getNumDatapoints());
    }
    m_camera.setYLimitsFromView();
    if (m_linkedSubplotCameraSettings.yAxisLimitMode == YAxisMode::FixedAuto)
    {
        updateYAxisLimits();
    }
}


void LinkedSubplot::updateYAxisLimits()
{
    double minY = m_JointPlotData.getDataMinY();
    double maxY = m_JointPlotData.getDataMaxY();

    double paddingConstProportion = 0.05;
    double padding = (maxY - minY) * paddingConstProportion;

    m_linkedSubplotCameraSettings.yAxisLimits = {minY - padding, maxY + padding};
}


/* Axis Limits
--------------------------------------------------------------------- */

void LinkedSubplot::setYLimits(std::optional<float> min, std::optional<float> max)
{
    // TODO: once the user has set, there is no way to go back to FixedAuto
    m_linkedSubplotCameraSettings.yAxisLimitMode = YAxisMode::FixedUser;

    if (min.has_value())
    {
        m_linkedSubplotCameraSettings.yAxisLimits[0] =  (double)min.value();
        m_camera.m_bottom = (double)min.value();
    }
    else
    {
        m_linkedSubplotCameraSettings.yAxisLimits[0] =  std::numeric_limits<double>::min();
    }

    if (max.has_value())
    {
        m_linkedSubplotCameraSettings.yAxisLimits[1] =  max.value();
        m_camera.m_top = (double)max.value();
    }
    else
    {
        m_linkedSubplotCameraSettings.yAxisLimits[1] =  std::numeric_limits<double>::max();
    }
}


void LinkedSubplot::pinYAxis(bool on)
{
    if (on)
    {
        m_linkedSubplotCameraSettings.yAxisLimitMode = YAxisMode::Pinned;
    }
    else
    {
        m_linkedSubplotCameraSettings.yAxisLimitMode = YAxisMode::FixedAuto;
    }
};


// Line Drawing
// -------------------------------------------------------------------------------- TODO TIDY THIS UP

void LinkedSubplot::startLineDraw(double x, double y, BackendDrawLineSettings drawLineSettings)
{
    // TOOD: handle cancelled halfway through drawing case!
    m_drawLines.emplace_back(std::make_unique<DrawLine>(m_gl, x, y, *this, drawLineSettings));
    m_lineDrawing = true;
}


void LinkedSubplot::mouseMoveLineDrawA(double x, double y)
{
    if (m_lineDrawing)
    {
        m_drawLines[m_drawLines.size() - 1]->handleMouseMove(x, y);
    }
}

void LinkedSubplot::endLineDraw()
{
    m_lineDrawing = false;
}


// Legend
// ---------------

void LinkedSubplot::setLegend(
    std::variant<std::vector<std::string>, std::vector<LegendItem>> labels,
    BackendLegendSettings legendSettings
)
{
    std::vector<LegendItemGlm> glmLabels;

    if (std::holds_alternative<std::vector<LegendItem>>(labels))
    {
        std::vector<LegendItem> labelsLegendItem = std::get<std::vector<LegendItem>>(labels);

        for (int i = 0; i < labelsLegendItem.size(); i++)
        {
            glmLabels.emplace_back(LegendItemGlm{labelsLegendItem[i]});
        }
    }
    else
    {
        std::vector<std::string> labelsString = std::get<std::vector<std::string>>(labels);

        const std::vector<std::unique_ptr<BasePlot>>& plotVector = m_JointPlotData.plotVector();

        for (int i = 0; i < plotVector.size(); i++)
        {
            // Guard the case when a plot is added after the
            // Legend is set.
            if (i < labelsString.size())
            {
                glmLabels.emplace_back(LegendItemGlm{labelsString[i], plotVector[i]});
            }
        }
    }

    m_legend = std::make_unique<Legend>(glmLabels, *this, m_configs, m_gl, legendSettings);
}
