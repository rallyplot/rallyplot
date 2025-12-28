#ifndef PLOTTER_H
#define PLOTTER_H

#include "UserVector.h"
#include <optional>
#include <variant>
#include <vector>
#include <string>
#include <memory>

#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <chrono>


// -------------------------------------------------------
// Custom Types
// -------------------------------------------------------

using StringVectorRef = std::reference_wrapper<const std::vector<std::string>>;
using TimepointVectorRef = std::reference_wrapper<const std::vector<std::chrono::system_clock::time_point>>;


using DateVector = std::variant<
    StringVectorRef,
    TimepointVectorRef
    >;


using ScatterDateVector = std::variant<
    StdPtrVector<int>,
    StringVectorRef,
    TimepointVectorRef
    >;


using OptionalDateVector = std::optional<
    DateVector
    >;

// -------------------------------------------------------
// Settings Structs
// -------------------------------------------------------

// Enums
// -------------------------------------------------------

enum class ColorMode
{
    light,
    dark
};


enum class Font
{
    arial,
    consola
};


/**
 * @brief Control the display of the candle plot.
 */
enum class CandlestickMode
{
    /** The full candlestick with capped shadows. */
    full,
    /** Draw only the candlestick body. */
    bodyOnly,
    /** Candlestick with uncapped shadows. */
    noCaps,
    /** A simple line plot of the 'open' data. */
    lineOpen,
    /** A simple line plot of the 'close' data. */
    lineClose
};


/**
 * @brief Shape of the scatter-plot markers.
 */
enum class ScatterShape
{
    circle,
    triangleUp,
    triangleDown,
    cross
};


enum class HoverValueDisplayMode
{
    alwaysShow,
    onlyUnderMouse,
    off
};


/**
 * @brief Return type for csv-loading function.
 */
struct CandleDataCSV
{
    std::vector<float> open;
    std::vector<float> high;
    std::vector<float> low;
    std::vector<float> close;
    std::vector<std::string> dates;
};


/**
 * @brief Settings that apply to the whole figure.
 */
struct PlotterArgs
{
    /** Width of the figure window. */
    int width = 800;

    /** Height of the figure window. */
    int height = 600;

    /** Light or dark mode. */
    ColorMode colorMode = ColorMode::light;

    /** Number of samples used in antialiasing smoothing. Zero is off, gives best performance.
        Higher looks smoother but is slower. Flickering of thin-line plots may occur when less than 4.*/
    int antiAliasingSamples = 0;

    /** Font to use for axis tick labels. */
    Font axisTickLabelFont = Font::arial;

    /** Point size for the axis tick labels. */
    int axisTickLabelFontSize = 12;

    /** If `true`, y-axis is located on the right of the plot. Otherwise, left of the plot. */
    bool axisRight = true;

    /** Size of margin between y-axis and the edge of the figure. */
    int widthMarginSize = 50;

    /** Size of the margin between the x-axis and the bottom edge of the figure. */
    int heightMarginSize = 25;

};


/**
 * @brief Controls plot pan and zooming.
 */
struct CameraSettings
{
    /** Controls the speed of zoom by key-press (Q, E, Z, C). */
    double keyZoomSpeed = 2.5;

    /** Controls the speed of zoom by mouse move (right-click). */
    double mouseZoomSpeed = 0.012;

    /** Controls the speed of panning by key-press (A, D, W, X). */
    double keyPanSpeed = 0.35;

    /** Controls the speed of panning by mouse move (left-click). */
    double mousePanSpeed = 0.70;

    /** Controls the speed of zoom by the mouse wheel. */
    double wheelSpeed = 0.002;

    /** If `true`, the axis zoom / pan is locked to the largest x-axis position. */
    bool lockMostRecentDate = false;

    /** All zooming will be fixed to the y-axis (right or left depending on PlotterArgs.axisRight). */
    bool fixZoomAtEdge = false;
};


/**
 * @brief Controls the axis display.
 */
struct AxisSettings

{
    /** Minimum number of ticks that should be displayed. Should be a multiple of 2.
     *  If too close to maxNumTicks, a flickering artifact may be observed. */
    int minNumTicks = 6;

    /** Maximum number of ticks that should be displayed. Should be a multiple of 2.
     *  If too close to minNumTicks, a flickering artifact may be observed. */
    int maxNumTicks = 12;

    /** Initial number of ticks, must be a multiple of two and between min / max num ticks. */
    int initNumTicks = 8;

    /** If `true`, small tick-lines will be displayed. */
    bool showTicks = true;

    /** Width of the tick line. May have no effect on some GPUs. */
    float tickLinewidth = 1.0f;

    /** Length of the tick line. */
    float tickSize = 0.025f;

    /** If `true`, grid lines (that extend the tick line across the plot) are displayed. */
    bool showGridline = true;

    /** Width of the grid line. May have no effect on some GPUs. */
    float gridlineWidth = 0.4f;

    /** Width of the axis line. May have no effect on some GPUs. */
    float axisLinewidth = 1.0f;

    /** Controls the number of decimal places for the y-axis tick labels. */
    std::optional<int> tickLabelDecimalPlaces = std::nullopt;   // X-axis does not have decimal places

    /** Grid line color, if `std::nullopt` uses the default according to ColorMode. */
    std::optional<std::vector<float>> gridlineColor = std::nullopt;

    /** Axis color, if `std::nullopt` uses the default according to ColorMode. */
    std::optional<std::vector<float>> axisColor = std::nullopt;

    /** Axis tick label color, if `std::nullopt` uses the default according to ColorMode. */
    std::optional<std::vector<float>> fontColor = std::nullopt;
};


struct XAxisSettings : public AxisSettings
{
    XAxisSettings() {};
};


struct YAxisSettings : public AxisSettings
/*
    Overwrite AxisSettings defaults for y-axis specific settings.
 */
{
    YAxisSettings() { initNumTicks = 12; tickLabelDecimalPlaces = 2; showTicks = false; };
};


/**
 * @brief Settings for a candlestick plot.
 */
struct CandlestickSettings
{
     /** Color for candles when close price is higher than open price. */
    std::vector<float> upColor = {8.0 / 255.0, 153.0 / 255.0, 129.0 / 255.0, 1.0};

     /** Color for candles when close price is lower than open price. */
    std::vector<float> downColor = {244.0 / 255.0, 52.0 / 255.0, 68.0 / 255.0, 1.0};

    /** Control how candles are displayed. */
    CandlestickMode mode = CandlestickMode::full;

     /** Ratio between candle and gap width, a float between (0, 1] e.g. 1 is no space between candles. */
    double candleWidthRatio = 0.75;

    /** Ratio between candle and cap width, a double between (0, 1] e.g. 1 the cap is the width of the candle. */
    double capWidthRatio = 0.5;

    /** Line width for open-line and close-line mode for the candlestick plot. */
    double lineModeLinewidth = 1.0;

     /** Miter limit controls the maximum line-segment connection length, for open-line and close-line mode.*/
    double lineModeMiterLimit = 3.0;

    /** If `true`, a simple line plot with fixd width is used (`width` and `miterLimit` have no effect). This is much faster.*/
    bool lineModeBasicLine = false;

};


/**
 * @brief Settings for a bar plot.
 */
struct BarSettings
{
    /** Color of the bar plot. */
    std::vector<float> color = {8.0 / 255.0, 153.0 / 255.0, 129.0 / 255.0, 1.0f};

    /** Ratio between the bar width and inter-bar gap, a double between (0, 1] e.g. 1 is no space between bars. */
    double widthRatio = 0.75;

    /** Minimum value of the bar plot. By default, the minimum number in `y` minus 1% of max y - min y as padding. */
    std::optional<float> minValue = std::nullopt;
};


/**
 * @brief Settings for a line plot.
 */
struct LineSettings
{
    /** Color of the line plot. */
    std::vector<float> color = {0.5f, 0.5f, 0.5f, 1.0f};

    /** Line width for the plot. */
    double width  = 0.5;

    /** Miter limit controls the maximum line-segment connection length, for open-line and close-line mode.*/
    double miterLimit = 3.0;

    /** If `true`, a simple line plot with fixd width is used (`width` and `miterLimit` have no effect). This is much faster.*/
    bool basicLine = false;
};


/**
 * @brief Settings for a scatter plot.
 */
struct ScatterSettings
{
    /** Shape of the scatter marker. */
    ScatterShape shape = ScatterShape::cross;

    /** Color of the scatter marker. */
    std::vector<float> color = {0.12f, 0.46f, 0.70f, 1.0f};

    /** If `true`, size of the scatter marker is the same at all zooms.
        If `false`, the size of the marker will decrease when zoomed out. */
    bool fixedSize = true;

    /** Size of the scatter marker when `fixedSize` is `true`. */
    double markerSizeFixed = 0.025;

    /** Size of the scatter marker when `fixedSize` if `false`. */
    double markerSizeFree = 10.0;
};


struct LabelSettingsBase
/*
    font :
    weight :
    fontSize :
    color :
*/
{
    Font font;
    std::string weight;
    int fontSize;
    std::optional<std::vector<float>> color;
};


/**
 * @brief Text settings for axis label.
 */
struct AxisLabelSettings : LabelSettingsBase
{
    AxisLabelSettings()
    {
        font = Font::arial;
        weight = "bold";
        fontSize = 12;
        std::optional<std::vector<float>> color = std::nullopt;
    }

    AxisLabelSettings(Font font_, std::string weight_, int fontSize_, std::optional<std::vector<float>> color_)
    {
        font = font_;
        weight = weight_;
        fontSize = fontSize_;
        color = color_;
    }
};


/**
 * @brief Text settings for the plot title.
 */
struct TitleLabelSettings : LabelSettingsBase
/*
    font :
    weight :
    fontSize :
    color :
*/
{
    TitleLabelSettings()
    {
        font = Font::arial;
        weight = "extrabold";
        fontSize = 12;
    }

    TitleLabelSettings(Font font_, std::string weight_, int fontSize_, std::optional<std::vector<float>> color_)
    {
        font = font_;
        weight = weight_;
        fontSize = fontSize_;
        color = color_;
    }
};


/**
 * @brief Settings for the plot legend.
 */
struct LegendSettings
{
    /** Simple way to scale the size of the legend. */
    float legendSizeScaler = 1.0f;

    /**  Padding between an outer box and the graph (x dimension). */
    float xBoxPad = 10.0f;

    /** Padding between an outer box and the the graph (y dimension). */
    float yBoxPad = 2.0f;

    /** Width of the colored line. */
    float lineWidth = 35.0;

    /** Height of the colored line (that indicates plot color). */
    float lineHeight = 7.5f;

    /**  Padding to the left of the colored line. */
    float xLinePadLeft = 5.0f;

    /** Padding to the right of the colored line. */
    float xLinePadRight = 5.0f;

    /** Padding to the right of the label text. */
    float xTextPadRight = 2.0f;

    /** Padding between label text elements (y dimension). */
    float yInterItemPad = 5.0f;

    /**  Padding above the first label text. */
    float yItemPad = 2.0f;

    /** Font for the label text. */
    Font font = Font::arial;

    /** Point size for the label text. */
    int fontSize = 15;

    /** Label text font color. Default is based on ColorMode. */
    std::optional<std::vector<float>> fontColor = std::nullopt;

    /** Color of the outer box (by default, invisible). */
    std::vector<float> boxColor = { 1.0, 1.0, 1.0, 0.0 };
};


/**
 * @brief Item to set both text label and bar color on the legend.
 */
struct LegendItem
{
    LegendItem(std::string name_, std::vector<float> leftColor_, std::vector<float> rightColor_) :
        name(name_), leftColor(leftColor_), rightColor(rightColor_) {};

    LegendItem(std::string name_, std::vector<float> color) :
        name(name_), leftColor(color), rightColor(color) {};

    /** Text label for the legend item. */
    std::string name;

    /** Left color for the color bar gradient. */
    std::vector<float> leftColor;

    /** Right color for the color bar gradient. */
    std::vector<float> rightColor;
};


/**
 * @brief Settings for the crosshair that follows the mouse over the plot.
 */
struct CrosshairSettings
{
    bool on = true;

    /** Font of the label that appears in the axis. */
    Font font = Font::arial;

    /**  Point size of the label that appears in the axis. */
    int fontSize = 10;

    /** Crosshair line width. */
    double linewidth = 1.0;

    /** Crosshair line color. Default is based on ColorMode. */
    std::optional<std::vector<float>> lineColor = std::nullopt;

    /** Background color of the label that appears in the axis. Default is based on ColorMode. */
    std::optional<std::vector<float>> backgroundColor = std::nullopt;

    /** Color of the font that appears in the axis. Default is based on ColorMode. */
    std::optional<std::vector<float>> fontColor = std::nullopt;
};


/**
 * @brief Settings for the pop-up label that appears on mouse hover over plot.
 */
struct HoverValueSettings
{
    /** Display mode ("alwaysShow", "onlyUnderMouse" (mouse must be over the plot value to show) or "off").*/
    HoverValueDisplayMode displayMode = HoverValueDisplayMode::alwaysShow;

    /** Font of the hover label. */
    Font font = Font::arial;

    /** Point size of the hover label. */
    int fontSize = 10;

    /** Font color of the hover label. Default is based on ColorMode. */
    std::optional<std::vector<float>> fontColor = std::nullopt;

    /** Background color of the hover label. Default is based on ColorMode.*/
    std::optional<std::vector<float>> backgroundColor = std::nullopt;

    /** Border color of the hover label. Default is based on ColorMode.*/
    std::optional<std::vector<float>> borderColor = std::nullopt;
};


/**
 * @brief Settings for lines drawn on the plot.
 */
struct DrawLineSettings
{
    /** Width of the drawn line. */
    float linewidth = 0.25f;

    /** Color of the drawn line. Default blue. */
    std::optional<std::vector<float>> color = std::nullopt;
};

// Plotter Class
// -------------------------------------------------------

#if defined(_WIN32) || defined(__CYGWIN__)
#define PLOTTER_API __declspec(dllexport)
#else
#define PLOTTER_API
#endif

class PLOTTER_API Plotter
/*
    Top-level class for plotting. Coordinate the plot window
    set up and all public plotting functions.

 */
{

public:

    /**
     * @brief The Plotter class controls all plotting.
     *
     * @param plotterArgs PlotterArgs struct of options to control the figure.
     */
    Plotter(
        std::optional<PlotterArgs> plotterArgs = std::nullopt
    );

    ~Plotter();

    Plotter(const Plotter&) = delete;
    Plotter& operator=(const Plotter&) = delete;
    Plotter(Plotter&&) = delete;
    Plotter& operator=(Plotter&&) = delete;

    /**
     * @brief Start the event loop to display and interact with plots.
     */
    void start();

    // Settings
    // ---------------------------------------------------------------------------------------------------------------

    /**
     * @brief Set the background color for the subplot.
     *
     * @param backgroundColor Vector of length 1 to 4 setting RGBA color values.
     * Missing values are set to 0.0f for color and 1.0f for alpha. e.g. {0.1f, 0.2f} sets {0.1f, 0.2f, 0.0f, 1.0f}.
     */
    void setBackgroundColor(const std::vector<float> backgroundColor);

    /**
     * @brief Set the camera settings that control pan and zooming.
     *
     * @param cameraSettings The CameraSettings struct of settings.
     * @param linkedSubplotIdx The linked subplot on which to set the settings. If `nullopt`, will be applied to all linked subplots.
     */
    void setCameraSettings(CameraSettings cameraSettings, std::optional<int> linkedSubplotIdx = std::nullopt);

    /**
     * @brief Settings for the crosshair that follows the mouse over the plot.
     */
    void setCrosshairSettings(CrosshairSettings crosshairSettings);

    /**
     * @brief Control how lines drawn on the plot are displayed.
     */
    void setDrawLineSettings(DrawLineSettings drawLineSettings);

    /**
     * @brief Control how the pop-up label that appears on mouse hover over plot is displayed.
     */
    void setHoverValueSettings(HoverValueSettings hoverValueSettings);

    /**
     * @brief Control how the x-axis is displayed.
     *
     * @param xAxisSettings XAxisSettings struct of axis settings.
     * @param linkedSubplotIdx The linked subplot on which to set the settings. If `nullopt`, will be applied to all linked subplots.
     */
    void setXAxisSettings(XAxisSettings xAxisSettings, std::optional<int> linkedSubplotIdx = std::nullopt);

    /**
     * @brief Control how the y-axis is displayed.
     *
     *
     * @param yAxisSettings YAxisSettings struct of axis settings.
     * @param linkedSubplotIdx The linked subplot on which to set the settings. If `nullopt`, will be applied to all linked subplots.
     */
    void setYAxisSettings(YAxisSettings yAxisSettings, std::optional<int> linkedSubplotIdx = std::nullopt);

    // Axis limit behaviour
    // ---------------------------------------------------------------------------------------------------------------

    /**
     * @brief Fix the y-axis zoom to the minimum and maximum values of the plot currently in view.
     *
     * @param on If `false`, y-axis is unpinned and can be zoomed freely.
     * @param linkedSubplotIdx The linked subplot to pin. If `nullopt`, will be applied to all linked subplots.
     */
    void pinYAxis(bool on = true, std::optional<int> linkedSubplotIdx = std::nullopt);

    /**
     * @brief Set minimum and / or maximum values for the y-axis. Only has an effect if pinYAxis is set to `false`.
     *
     * @param min Minimum value for the y-axis, if `nullopt` is set to -inf.
     * @param max Maximum value for the y-axis, if `nullopt` is set to inf.
     * @param linkedSubplotIdx The linked subplot on which to limit the y-axis view. If `nullopt`, will be applied to all linked subplots.
     */
    void setYLimits(
        std::optional<double> min,
        std::optional<double> max,
        std::optional<int> linkedSubplotIdx = std::nullopt
    );

    /**
     * @brief Set the minimum and maximum values for the x-axis. Must be the same type
     * as the x-axis data. x-axis values should be unique.
     */
    void setXLimits(
        std::optional<std::variant<int, std::string, std::chrono::system_clock::time_point>> min,
        std::optional<std::variant<int, std::string, std::chrono::system_clock::time_point>> max
    );

    /**
     * @brief Link the y-axis across all linked subplots.
     *
     * @param on If `true`, zooming on one plot will zoom on all linked subplots.
     */
    void linkYAxis(bool on = true);

    // Labels and legend
    // ---------------------------------------------------------------------------------------------------------------

    /**
     * @brief Set a label on the y-axis for the subplot.
     *
     * @param text Text to set as the label.
     * @param settings AxisLabelSettings that control the display of the label.
     */
    void setYLabel(std::string text, std::optional<AxisLabelSettings> settings = std::nullopt);

    /**
     * @brief Set a label on the x-axis for the subplot.
     *
     *
     * @param text Text to set as the label.
     * @param settings AxisLabelSettings that control the display of the label. If `nullopt`, defaults are used.
     */
    void setXLabel(std::string text, std::optional<AxisLabelSettings> settings = std::nullopt);

    /**
     * @brief Set a title on the subplot.
     *
     *
     * @param text Text to set as the title.
     * @param settings TitleLabelSettings that control the display of the label.
     */
    void setTitle(std::string text, std::optional<TitleLabelSettings> settings = std::nullopt);

    /**
     * @brief Set a legend on linked subplots.
     *
     *
     * @param labelNames A vector of strings to set as the legend item labels.
     * @param legendSettings LegendSettings struct that controls the display of the legend. If `nullopt`, defaults are used.
     * @param linkedSubplotIdx The linked subplot to set the legend on. By default, it is the most recently added linked subplot.
     */
    void setLegend(
        std::vector<std::string> labelNames, std::optional<LegendSettings> legendSettings = std::nullopt, int linkedSubplotIdx = -1
    );

    /**
     * @brief Set a legend on linked subplots.
     *
     * @param labelItems A vector of LegendItems, where LegendItem contains a label string and colors to set on the item bar.
     * @param legendSettings LegendSettings struct that controls the display of the legend. If `nullopt`, defaults are used.
     * @param linkedSubplotIdx The linked subplot to set the legend on. By default, it is the most recently added linked subplot.
     */
    void setLegend(
        std::vector<LegendItem> labelItems, std::optional<LegendSettings> legendSettings = std::nullopt, int linkedSubplotIdx = -1
    );

    // Subplots
    // ---------------------------------------------------------------------------------------------------------------

    /**
     * @brief Add a new subplot to the figure. After adding, all calls on Plotter will operate on the
     * added subplot. To change, use setActiveSubplot.
     *
     * Specify the row and col index at which to add the subplot, as well as how many
     * rows and columns it spans. For example, to add a second subplot when there is 1 existing plot:
     * (0, 1, 1, 1) will add a new subplot to the right of the existing plot.
     * (1, 0, 1, 1) will add the new subplot underneath the existing plot.
     *
     * Note that a 'subplot' is a different entity to a 'linked subplot'.
     * All linked subplots share the x-axis, while a subplot is a completely separate set of plots.
     *
     * @param row Index of the row in which to add the subplot.
     * @param col Index of the column in which to add the subplot.
     * @param rowSpan Number of rows the subplot will span.
     * @param colSpan Number of columns the subplot will span.
     */
    void addSubplot(int row, int col, int rowSpan = 1, int colSpan = 1);

    /**
     * @brief Set the 'active' subplot to the subplot at index row, col.
     * All calls to Plotter e.g. Plotter.candlestick() operate on the active subplot.
     */
    void setActiveSubplot(int row, int col);

    /**
     * @brief Add a linked subplot to the subplot.
     *
     * A 'linked subplot' is a subplot that shares the x-axis with the current plot.
     * This is in contrast to a 'subplot' which is an entirely separate plot.
     *
     *
     * @param heightAsProportion The height of the new linked subplot as a proportion of the plot (in range [0, 1]).
     */
    void addLinkedSubplot(double heightAsProportion);

    /**
     * @brief Resize the linked subplots.
     *
     *
     * @param yHeights A vector of height proportions. Length must equal the number of
     * linked subplots on the plot and must sum to 1.
     */
    void resizeLinkedSubplots(std::vector<double> yHeights);

    // Plots
    // ---------------------------------------------------------------------------------------------------------------

    /**
     * @brief Add a candlestick plot.
     *
     *
     * @param open Vector of candle open prices.
     * @param high Vector of candle high prices.
     * @param low Vector of candle low prices.
     * @param close Vector of candle close prices.
     * @param candlestickSettings
     * @param linkedSubplotIdx The index of the linked subplot on which to plot the candlesticks. By default, it is the most recently added linked subplot.
     */
    void candlestick(
        const std::vector<float>& open,
        const std::vector<float>& high,
        const std::vector<float>& low,
        const std::vector<float>& close,
        std::optional<CandlestickSettings> candlestickSettings = std::nullopt,
        int linkedSubplotIdx = -1
        );

    /**
     * @brief  Add a candlestick plot with string x-labels.
     *
     * @param open Vector of candle open prices.
     * @param high Vector of candle high prices.
     * @param low Vector of candle low prices.
     * @param close Vector of candle close prices.
     * @param dates Vector of strings used for x-axis tick labels. Must be same size as other vector inputs.
     * @param candlestickSettings
     * @param linkedSubplotIdx The index of the linked subplot on which to plot the candlesticks. By default, it is the most recently added linked subplot.
    */
    void candlestick(
        const std::vector<float>& open,
        const std::vector<float>& high,
        const std::vector<float>& low,
        const std::vector<float>& close,
        const std::vector<std::string>& dates,
        std::optional<CandlestickSettings> candlestickSettings = std::nullopt,
        int linkedSubplotIdx = -1
    );

    /**
     * @brief  Add a candlestick plot with UTC chrono-timepoints as x-labels.
     *
     * @param open Vector of candle open prices.
     * @param high Vector of candle high prices.
     * @param low Vector of candle low prices.
     * @param close Vector of candle close prices.
     * @param dates Vector of chrono::timepoints (UTC) used for x-axis tick labels. Must be same size as other vector inputs.
     * @param candlestickSettings
     * @param linkedSubplotIdx The index of the linked subplot on which to plot the candlesticks. By default, it is the most recently added linked subplot.
    */
    void candlestick(
        const std::vector<float>& open,
        const std::vector<float>& high,
        const std::vector<float>& low,
        const std::vector<float>& close,
        const std::vector<std::chrono::system_clock::time_point>& dates,
        std::optional<CandlestickSettings> candlestickSettings = std::nullopt,
        int linkedSubplotIdx = -1
    );

    /**
     * @brief Add a candlestick plot.
     *
     * This is mostly for internal use and instead of std::vector<float>&
     * takes pointers and size to such an array.
     *
     * @param openPtr Pointer to array of floats of open prices.
     * @param openSize Size of the open prices array.
     * @param highPtr  Pointer to array of floats of high prices..
     * @param highSize Size of the high prices array.
     * @param lowPtr  Pointer to array of floats of low prices.
     * @param lowSize Size of the low prices array.
     * @param closePtr  Pointer to array of floats of close prices.
     * @param closeSize Size of the close prices array.
     * @param dates Array of string or timepoints used as x-axis labels. If `nullopt`, integers starting at 0 are used.
     * @param candlestickSettings
     * @param linkedSubplotIdx The index of the linked subplot on which to plot the candlesticks. By default, it is the most recently added linked subplot.
     */
    void candlestick(
        const float* openPtr, std::size_t openSize,
        const float* highPtr, std::size_t highSize,
        const float* lowPtr, std::size_t lowSize,
        const float* closePtr, std::size_t closeSize,
        const OptionalDateVector dates = std::nullopt,
        std::optional<CandlestickSettings> candlestickSettings = std::nullopt,
        int linkedSubplotIdx = -1
    );

    /**
     * @brief Add a line plot.
     *
     *
     * @param yData Vector of float datapoints to plot.
     * @param dates Vector of string or chrono::timepoint (UTC) to use as x tick labels.
     * @param lineSettings
     * @param linkedSubplotIdx The index of the linked subplot on which to plot the line plot. By default, it is the most recently added linked subplot.
     */
    void line(
        const std::vector<float>& yData,
        const OptionalDateVector dates,
        std::optional<LineSettings> lineSettings = std::nullopt,
        int linkedSubplotIdx = -1
    );

    /**
     * @brief line
     *
     * @param yData Vector of float datapoints to plot.
     * @param lineSettings
     * @param linkedSubplotIdx The index of the linked subplot on which to plot the line plot. By default, it is the most recently added linked subplot.
     */
    void line(
        const std::vector<float>& yData,
        std::optional<LineSettings> lineSettings = std::nullopt,
        int linkedSubplotIdx = -1
    );

    /**
     * @brief Add a line plot.
     *
     * @param yPtr Pointer to a vector of floats to plot.
     * @param ySize Size of the float vector.
     * @param dates string or chrono::timepoint (UTC) to use as x-tick labels. If `nullopt`, integers starting at 0 are used.
     * @param lineSettings
     * @param linkedSubplotIdx The index of the linked subplot on which to plot the line plot. By default, it is the most recently added linked subplot.
     */
    void line(
        const float* yPtr,
        std::size_t ySize,
        const  OptionalDateVector dates = std::nullopt,
        std::optional<LineSettings> lineSettings = std::nullopt,
        int linkedSubplotIdx = -1
        );

    /**
     * @brief Add a bar plot.
     *
     * @param yData Vector of float datapoints to plot.
     * @param dates Vector of string or chrono::timepoint (UTC) to use as x tick labels.
     * @param barSettings
     * @param linkedSubplotIdx The index of the linked subplot on which to plot the bar plot. By default, it is the most recently added linked subplot.
     */
    void bar(
        const std::vector<float>& yData,
        const OptionalDateVector dates,
        std::optional<BarSettings> barSettings = std::nullopt,
        int linkedSubplotIdx = -1
        );

    /**
     * @brief Add a bar plot.
     *
     * @param yData Vector of float datapoints to plot.
     * @param barSettings
     * @param linkedSubplotIdx The index of the linked subplot on which to plot the bar plot. By default, it is the most recently added linked subplot.
     */
    void bar(
        const std::vector<float>& yData,
        std::optional<BarSettings> barSettings = std::nullopt,
        int linkedSubplotIdx = -1
    );

    /**
     * @brief Add a bar plot.
     *
     * @param yPtr Pointer to a vector of floats to plot.
     * @param ySize Size of the float vector.
     * @param dates string or chrono::timepoint (UTC) to use as x-tick labels. If `nullopt`, integers starting at 0 are used.
     * @param barSettings
     * @param linkedSubplotIdx The index of the linked subplot on which to plot the bar plot. By default, it is the most recently added linked subplot.
     */
    void bar(
        const float* yPtr,
        std::size_t ySize,
        const OptionalDateVector xData = std::nullopt,
        std::optional<BarSettings> barSettings = std::nullopt,
        int linkedSubplotIdx = -1
        );

    /**
     * @brief Add a scatter plot.
     *
     * A scatter plot cannot be the first plot, it must be overlaid onto an existing plot.
     *
     * @param xData A vector of chrono::time_point (UTC) of x-axis position of the scatter points. The type must match the current x tick label type.
     * @param yData Vector of floats containing y-axis data. Must match xData in length.
     * @param scatterSettings
     * @param linkedSubplotIdx The index of the linked subplot on which to plot the scatter plot. By default, it is the most recently added linked subplot.
     */
    void scatter(
        const std::vector<std::chrono::system_clock::time_point>& xData,
        const std::vector<float>& yData,
        std::optional<ScatterSettings> scatterSettings = std::nullopt,
        int linkedSubplotIdx = -1
    );

    /**
     * @brief scatter
     *
     * A scatter plot cannot be the first plot, it must be overlaid onto an existing plot.
     *
     * @param xData A vector of chrono::time_point (UTC) of x-axis position of the scatter points. The type must match the current x tick label type.
     * @param yPtr Pointer to vector of floats containing y-axis data.
     * @param ySize Size of vector of floats yPtr points to. Must match xData in length.
     * @param scatterSettings
     * @param linkedSubplotIdx The index of the linked subplot on which to plot the scatter plot. By default, it is the most recently added linked subplot.
     */
    void scatter(
        const std::vector<std::chrono::system_clock::time_point>& xData,
        const float* yPtr,
        std::size_t ySize,
        std::optional<ScatterSettings> scatterSettings = std::nullopt,
        int linkedSubplotIdx = -1
        );

    /**
     * @brief scatter
     *
     *
     * @param xData A vector of strings of x-axis position of the scatter points. The type must match the current x tick label type.
     * @param yData Vector of floats containing y-axis data. Must match xData in length.
     * @param scatterSettings
     * @param linkedSubplotIdx The index of the linked subplot on which to plot the scatter plot. By default, it is the most recently added linked subplot.
     */
    void scatter(
        const std::vector<std::string>& xData,
        const std::vector<float>& yData,
        std::optional<ScatterSettings> scatterSettings = std::nullopt,
        int linkedSubplotIdx = -1
    );

    /**
     * @brief scatter
     *
     *
     * @param xData A vector of strings of x-axis position of the scatter points. The type must match the current x tick label type.
     * @param yPtr Pointer to vector of floats containing y-axis data.
     * @param ySize Size of vector of floats yPtr points to. Must match xData in length.
     * @param scatterSettings
     * @param linkedSubplotIdx The index of the linked subplot on which to plot the scatter plot. By default, it is the most recently added linked subplot.
     */
    void scatter(
        const std::vector<std::string>& xData,
        const float* yPtr,
        std::size_t ySize,
        std::optional<ScatterSettings> scatterSettings = std::nullopt,
        int linkedSubplotIdx = -1
        );

    /**
     * @brief scatter
     *
     *
     * @param xData A vector of integers of x-axis position of the scatter points. The type must match the current x tick label type.
     * @param yData Vector of floats containing y-axis data. Must match xData in length.
     * @param scatterSettings
     * @param linkedSubplotIdx The index of the linked subplot on which to plot the scatter plot. By default, it is the most recently added linked subplot.
     */
    void scatter(
        const std::vector<int>& xData,
        const std::vector<float>& yData,
        std::optional<ScatterSettings> scatterSettings = std::nullopt,
        int linkedSubplotIdx = -1
    );

    /**
     * @brief scatter
     *
     *
     * @param xPtr A pointer to a vector of integers of x-axis position of the scatter points. The type must match the current x tick label type.
     * @param xSize Size of vector at xPtr.
     * @param yPtr Pointer to vector of floats containing y-axis data.
     * @param ySize Size of vector of floats yPtr points to. Must match xData in length.
     * @param scatterSettings
     * @param linkedSubplotIdx The index of the linked subplot on which to plot the scatter plot. By default, it is the most recently added linked subplot.
     */
    void scatter(
        const int* xPtr,
        std::size_t xSize,
        const float* yPtr,
        std::size_t ySize,
        std::optional<ScatterSettings> scatterSettings = std::nullopt,
        int linkedSubplotIdx = -1
    );

    /**
     * @brief Read data from a csv file.
     *
     * Requires a .csv with "Open", "Close", "Low", "High" columns and optional "Date" column.
     *
     *
     * @param dataFilepath Path to the .csv file.
     */
    CandleDataCSV _readDataFromCSV(const std::string& dataFilepath) const;

    std::tuple<std::vector<std::uint8_t>, int, int> _grabFrameBuffer(
        std::optional<int> row = std::nullopt, std::optional<int> col = std::nullopt
    );

private:

    class Impl;
    std::unique_ptr<Impl> pImpl;
};


#endif
