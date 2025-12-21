#pragma once

#include <cmath>
#include <string>
#include <glm.hpp>
#include <QString>
#include "include/Plotter.h"
#include "Utils.h"

// Be very careful when editing these as set in order of definition below.

constexpr int cfg_MAX_NUM_TICKS = 50;


struct ColorModeDefaults {
    virtual ~ColorModeDefaults() = default;

    glm::vec4 backgroundColor;
    glm::vec4 axisColor;
    glm::vec4 gridlineColor;
    glm::vec4 fontColor;
};


struct CustomModeDefaults : public ColorModeDefaults
{
    CustomModeDefaults()
    {
        backgroundColor = glm::vec4(0.67f, 0.23f, 0.56f, 1.0f);
        axisColor = glm::vec4(0.67f, 0.23f, 0.56f, 1.0f);
        gridlineColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        fontColor = glm::vec4(0.67f, 0.23f, 0.56f, 1.0f);
    }
};


struct LightModeDefaults : public ColorModeDefaults
{
    LightModeDefaults()
    {
        backgroundColor = glm::vec4(0.9999f, 0.9999f, 0.9999f, 1.0f);
        axisColor = glm::vec4(0.290f, 0.290f, 0.290f, 1.0f);
        gridlineColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        fontColor = glm::vec4(0.290f, 0.290f, 0.290f, 1.0f);
    }
};


struct DarkModeDefaults : public ColorModeDefaults
{
    DarkModeDefaults()
    {
        backgroundColor = glm::vec4(0.122f, 0.122f, 0.122f, 1.0f);
        axisColor = glm::vec4(0.843f, 0.843f, 0.843f, 1.0f);
        gridlineColor = glm::vec4(0.251f, 0.251f, 0.251f, 1.0f);
        fontColor = glm::vec4(0.843f, 0.843f, 0.843f, 1.0f);
    }
};

// This is a direct copy of AxisSettings but the
// vector type is changed to GLM to keep that dependency hidden.
// This isn't ideal.
struct BackendAxisSettings
{
    int minNumTicks;
    int maxNumTicks;
    int initNumTicks;
    bool showTicks;
    float tickLinewidth;
    float tickSize;
    bool showGridline;
    float gridlineWidth;

    float axisLinewidth;
    std::optional<int> tickLabelDecimalPlaces;
    glm::vec4 gridlineColor;
    glm::vec4 axisColor;
    glm::vec4 fontColor;
};

// The public interface splits basic camera settings
// and y axis limits / zoom mode. Under the hood however
// these are combined. Take the default arguments from the
// public interface and by default set no y-axis limits.

//
// Pinned : automatically pinned to the min / max value in the view
// Fixed : Set to fixed values based on the plot data
// UserSet : fixed, and set by user.
enum class YAxisMode {Pinned, FixedAuto, FixedUser};


struct BackendCameraSettings
{
    CameraSettings core;
    // by default, yAxisLimits are auto-calculated
    // on load based on the data (with some padding)
    // axis-mode ("pin", "default", "set")
    // bool pinYAxis = false;
    // yAxisLimitsOn
    YAxisMode yAxisLimitMode = YAxisMode::Pinned;
    std::array<double, 2> yAxisLimits = { NAN, NAN } ;
};


struct PlotOptions
{
    ColorMode colorMode;
    Font axisTickLabelFont;
    int axisTickLabelFontSize;
    bool axisRight;
    int widthMarginSize;
    int heightMarginSize;
    int initWidth;
    int initHeight;
};


// TODO: this approach is insane. There must be a better way!
// ------------------------------------------------------------------

struct BackendCandlestickSettings
{
    BackendCandlestickSettings(const CandlestickSettings& settings)
    {
        candleWidthRatio = settings.candleWidthRatio;
        capWidthRatio = settings.capWidthRatio;
        upColor = utils_colorToGlmVec(settings.upColor.data(), settings.upColor.size());
        downColor = utils_colorToGlmVec(settings.downColor.data(), settings.downColor.size());
        mode = settings.mode;
        lineModeLinewidth = settings.lineModeLinewidth;
        lineModeMiterLimit = settings.lineModeMiterLimit;
        lineModeBasicLine = settings.lineModeBasicLine;
    }
    double candleWidthRatio;
    double capWidthRatio;
    glm::vec4 upColor;
    glm::vec4 downColor;
    CandlestickMode mode;
    double lineModeLinewidth;
    double lineModeMiterLimit;
    bool lineModeBasicLine;
};


struct BackendLegendSettings
/*
    Determines how the legend appears on the plot.

    Currently, the legend is fixed to the top-right. The colored lines
    are pinned to the middle of the text label. The by default invisible)
    outer box size is deterministic.
 */
{
    float legendSizeScaler;
    float xBoxPad;
    float yBoxPad;
    float lineWidth;
    float lineHeight;
    float xLinePadLeft;
    float xLinePadRight;
    float xTextPadRight;
    float yInterItemPad;
    float yItemPad;
    Font font;
    int fontSize;
    glm::vec4 fontColor;
    glm::vec4 boxColor;
};


struct BackendBarSettings
{
    BackendBarSettings(const BarSettings& settings)
    {
        color = utils_colorToGlmVec(settings.color.data(), settings.color.size());
        widthRatio = settings.widthRatio;
        minValue = settings.minValue;
    }
    glm::vec4 color;
    double widthRatio;
    std::optional<float> minValue;
};


struct BackendLineSettings
{
    BackendLineSettings(const LineSettings& settings)
    {
        color = utils_colorToGlmVec(settings.color.data(), settings.color.size());
        width = settings.width;
        miterLimit = settings.miterLimit;
        basicLine = settings.basicLine;
    }
    glm::vec4 color;
    double width;
    double miterLimit;
    bool basicLine;
};


struct BackendScatterSettings
{
    BackendScatterSettings(const ScatterSettings& settings)
    {
        shape = settings.shape;
        color = utils_colorToGlmVec(settings.color.data(), settings.color.size());
        fixedSize = settings.fixedSize;
        markerSizeFixed = settings.markerSizeFixed;
        markerSizeFree = settings.markerSizeFree;
    }
    ScatterShape shape;
    glm::vec4 color;               // color
    bool fixedSize;          // fixedSize
    double markerSizeFixed;        // markerSizeFixed;
    double markerSizeFree;         // markerSizeFree
};


struct BackendCrosshairSettings
{
    bool on;
    Font font;
    int fontSize;
    double linewidth;
    glm::vec4 backgroundColor;
    glm::vec4 fontColor;
    glm::vec4 lineColor;
};


struct BackendDrawLineSettings
{
    double linewidth;
    glm::vec4 color;
};


struct BackendHoverValueSettings
{
    HoverValueDisplayMode displayMode;
    Font font;
    int fontSize;
    glm::vec4 fontColor;
    glm::vec4 backgroundColor;
    glm::vec4 borderColor;
};


class Configs
/*
    TODO: explain levels
 */
{

public:
    Configs(PlotOptions plotOptions)
    : m_plotOptions(plotOptions)
    {
        m_backgroundColor = m_plotOptions.colorMode == ColorMode::light ? m_lightModeDefaults.backgroundColor : m_darkModeDefaults.backgroundColor;
        renderColorMode();

        m_defaultCrosshairSettings = convertToCrosshairSettings(CrosshairSettings{});
        m_defaultDrawLineSettings = convertToBackendDrawLineSettings(DrawLineSettings{});
        m_defaultHoverValueSettings = convertBackendHoverValueSettings(HoverValueSettings{});
        m_defaultXAxisSettings = convertToBackendAxisSettings(XAxisSettings{});
        m_defaultYAxisSettings = convertToBackendAxisSettings(YAxisSettings{});
    };
    ~Configs() {};

    BackendCrosshairSettings m_defaultCrosshairSettings;
    BackendDrawLineSettings m_defaultDrawLineSettings;
    BackendHoverValueSettings m_defaultHoverValueSettings;
    BackendAxisSettings m_defaultXAxisSettings;
    BackendAxisSettings m_defaultYAxisSettings;
    BackendCameraSettings m_defaultCameraSettings;

    DarkModeDefaults m_darkModeDefaults;
    LightModeDefaults m_lightModeDefaults;
    CustomModeDefaults m_customModeDefaults;

    BackendCrosshairSettings convertToCrosshairSettings(CrosshairSettings crosshairSettings);
    BackendDrawLineSettings convertToBackendDrawLineSettings(DrawLineSettings drawLineSettings);
    BackendAxisSettings convertToBackendAxisSettings(AxisSettings axisSettings);
    BackendHoverValueSettings convertBackendHoverValueSettings(HoverValueSettings hoverValueSettings);
    BackendLegendSettings convertBackendLegendSettings(LegendSettings legendSettings);

    void renderColorMode();

    PlotOptions m_plotOptions;
    glm::vec4 m_backgroundColor;

    // These are not exposed.
    int m_numStartingDatapoints = 150;
    double m_xAxisPadding = 0.75;  // cannot be more than 1

    // shared X axis configs
    bool m_xAxisLimitsOn = false;
    std::vector<double> m_xAxisLimits = {0.0, 0.0};

    bool m_linkYZoomAndPan = false;

};
