#include "LineData.h"


LineData::LineData(Configs& configs, const float* yPtr, std::size_t ySize)
    : m_configs(configs), m_yData(yPtr, ySize)
{

    m_numDataPoints = m_yData.size();
    m_delta = 1.0 / m_numDataPoints;
}


std::optional<UnderMouseData> LineData::getDataUnderMouse(
    int _,
    double yData,
    double yPadding,
    bool alwaysShow,
    std::optional<double> xMousePos
) const
/*
    For LineData, it is not enough to get the data at the mouse tip to the nearest index,
    as it is only the value of the point, not the interpolation. Here we must linear
    interpolate between the two nearest points oto the mouse in order to get the exact value.
*/
{
    double xMousePosValue = xMousePos.value();
    if (xMousePosValue < 0.0)
    {
        xMousePosValue = 0.0;
    }
    int idxLower = std::floor(xMousePosValue / m_delta);
    int idxUpper = idxLower + 1;

    if (idxUpper > m_yData.size() - 1)
    {
        idxUpper = m_yData.size() - 1;
    }

    double m = (m_yData[idxUpper] - m_yData[idxLower]) / m_delta;

    double xStart = idxLower * m_delta;
    double yPlotData =  m_yData[idxLower] + (xMousePos.value() - xStart) * m;

    if ((alwaysShow || yPlotData - yPadding < yData && yData < yPlotData + yPadding))
    {
        return UnderMouseData {
            yPlotData
        };
    }
    else
    {
        return std::nullopt;
    }
}
