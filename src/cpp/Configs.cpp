/*
    These functions convert from frontend API values to backend. This handles two main cases:
        - backend axis settings use glm::vec4 instead of std::vector. However, we don't want
          glm as an external dependency, so perform the conversion internally here.
        - Default colors for light / dark mode, we want to set these according to
          the light / dark mode, and only set them differently if the user passes a value.
*/
#include "Configs.h"


BackendAxisSettings Configs::convertToBackendAxisSettings(const AxisSettings& axisSettings)
/*
    This assumes setYAxisSettings / setXAxisSettings is only called once
    (because, if setYAxisSettings for example is called twice, the defaults
    will override the settings from the first call. Handling this will be much
    more complex.
 */
{
    BackendAxisSettings settings;

    settings.minNumTicks = axisSettings.minNumTicks;
    settings.maxNumTicks = axisSettings.maxNumTicks;
    settings.initNumTicks = axisSettings.initNumTicks;
    settings.showTicks = axisSettings.showTicks;
    settings.tickLinewidth = axisSettings.tickLinewidth;
    settings.tickSize = axisSettings.tickSize;
    settings.showGridline = axisSettings.showGridline;
    settings.gridlineWidth = axisSettings.gridlineWidth;
    settings.axisLinewidth = axisSettings.axisLinewidth;
    settings.tickLabelDecimalPlaces = axisSettings.tickLabelDecimalPlaces;

    if (axisSettings.gridlineColor.has_value())
    {
        settings.gridlineColor = utils_colorToGlmVec(axisSettings.gridlineColor.value().data(), axisSettings.gridlineColor.value().size());
    }
    else
    {
        settings.gridlineColor = (m_plotOptions.colorMode == ColorMode::light) ? m_lightModeDefaults.gridlineColor : m_darkModeDefaults.gridlineColor;
    }

    if (axisSettings.axisColor.has_value())
    {
        settings.axisColor = utils_colorToGlmVec(axisSettings.axisColor.value().data(), axisSettings.axisColor.value().size());
    }
    else
    {
        settings.axisColor = (m_plotOptions.colorMode == ColorMode::light) ? m_lightModeDefaults.axisColor : m_darkModeDefaults.axisColor;
    }

    if (axisSettings.fontColor.has_value())
    {
        settings.fontColor = utils_colorToGlmVec(axisSettings.fontColor.value().data(), axisSettings.fontColor.value().size());
    }
    else
    {
        settings.fontColor = (m_plotOptions.colorMode == ColorMode::light) ? m_lightModeDefaults.fontColor : m_darkModeDefaults.fontColor;
    }

    return settings;
}


BackendCrosshairSettings Configs::convertToCrosshairSettings(const CrosshairSettings& crosshairSettings)
{
    BackendCrosshairSettings settings;

    settings.on = crosshairSettings.on;
    settings.font = crosshairSettings.font;
    settings.fontSize = crosshairSettings.fontSize;
    settings.linewidth = crosshairSettings.linewidth;

    if (crosshairSettings.lineColor.has_value())
    {
        settings.lineColor = utils_colorToGlmVec(crosshairSettings.lineColor.value().data(), crosshairSettings.lineColor.value().size());
    }
    else
    {
        settings.lineColor = (m_plotOptions.colorMode == ColorMode::light) ? m_lightModeDefaults.fontColor : m_darkModeDefaults.fontColor;
    }

    if (crosshairSettings.backgroundColor.has_value())
    {
        settings.backgroundColor = utils_colorToGlmVec(crosshairSettings.backgroundColor.value().data(), crosshairSettings.backgroundColor.value().size());
    }
    else
    {
        settings.backgroundColor = (m_plotOptions.colorMode == ColorMode::light) ? m_lightModeDefaults.backgroundColor : m_darkModeDefaults.backgroundColor;
    }

    if (crosshairSettings.fontColor.has_value())
    {
        settings.fontColor = utils_colorToGlmVec(crosshairSettings.fontColor.value().data(), crosshairSettings.fontColor.value().size());
    }
    else
    {
        settings.fontColor = (m_plotOptions.colorMode == ColorMode::light) ? m_lightModeDefaults.fontColor : m_darkModeDefaults.fontColor;
    }

    return settings;
}


BackendDrawLineSettings Configs::convertToBackendDrawLineSettings(const DrawLineSettings& drawLineSettings)
{
    BackendDrawLineSettings settings;

    settings.linewidth = drawLineSettings.linewidth;

    if (drawLineSettings.color.has_value())
    {
        settings.color = utils_colorToGlmVec(drawLineSettings.color.value().data(), drawLineSettings.color.value().size());
    }
    else
    {
        settings.color = (m_plotOptions.colorMode == ColorMode::light) ? m_lightModeDefaults.fontColor : m_darkModeDefaults.fontColor;
    }

    return settings;
}

BackendHoverValueSettings Configs::convertBackendHoverValueSettings(const HoverValueSettings& hoverValueSettings)
{
    BackendHoverValueSettings settings;

    settings.displayMode = hoverValueSettings.displayMode;
    settings.font = hoverValueSettings.font;
    settings.fontSize = hoverValueSettings.fontSize;

    if (hoverValueSettings.backgroundColor.has_value())
    {
        settings.backgroundColor = utils_colorToGlmVec(hoverValueSettings.backgroundColor.value().data(), hoverValueSettings.backgroundColor.value().size());
    }
    else
    {
        settings.backgroundColor = (m_plotOptions.colorMode == ColorMode::light) ? m_lightModeDefaults.backgroundColor : m_darkModeDefaults.backgroundColor;
    }

    if (hoverValueSettings.borderColor.has_value())
    {
        settings.borderColor = utils_colorToGlmVec(hoverValueSettings.borderColor.value().data(), hoverValueSettings.borderColor.value().size());
    }
    else
    {
        settings.borderColor = (m_plotOptions.colorMode == ColorMode::light) ? m_lightModeDefaults.fontColor : m_darkModeDefaults.fontColor;
    }

    if (hoverValueSettings.fontColor.has_value())
    {
        settings.fontColor = utils_colorToGlmVec(hoverValueSettings.fontColor.value().data(), hoverValueSettings.fontColor.value().size());
    }
    else
    {
        settings.fontColor = (m_plotOptions.colorMode == ColorMode::light) ? m_lightModeDefaults.fontColor : m_darkModeDefaults.fontColor;
    }

    return settings;
}


BackendLegendSettings Configs::convertBackendLegendSettings(const LegendSettings& legendSettings)
{
    BackendLegendSettings settings;

    settings.legendSizeScaler = legendSettings.legendSizeScaler;
    settings.xBoxPad = legendSettings.xBoxPad;
    settings.yBoxPad = legendSettings.yBoxPad;
    settings.lineWidth = legendSettings.lineWidth;
    settings.lineHeight = legendSettings.lineHeight;
    settings.xLinePadLeft = legendSettings.xLinePadLeft;
    settings.xLinePadRight = legendSettings.xLinePadRight;
    settings.xTextPadRight = legendSettings.xTextPadRight;
    settings.yInterItemPad = legendSettings.yInterItemPad;
    settings.yItemPad =legendSettings.yItemPad;
    settings.font = legendSettings.font;;
    settings.fontSize = legendSettings.fontSize;
    settings.boxColor = utils_colorToGlmVec(legendSettings.boxColor.data(), legendSettings.boxColor.size());

    if (legendSettings.fontColor.has_value())
    {
        settings.fontColor = utils_colorToGlmVec(legendSettings.fontColor.value().data(), legendSettings.fontColor.value().size());
    }
    else
    {
        settings.fontColor = (m_plotOptions.colorMode == ColorMode::light) ? m_lightModeDefaults.fontColor : m_darkModeDefaults.fontColor;
    }

    return settings;
};

