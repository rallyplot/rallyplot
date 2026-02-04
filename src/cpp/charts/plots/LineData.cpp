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
    assert(xMousePos.has_value());
    assert(m_yData.size() >= 2);

    // Given a mouse position value (known between 0 and some value), find the data index (lower)
    // and index + 1 (higher) as the indices of the points to interpolate.
    // Finally that the calculated index is not larger than the data.
    double xMousePosValue = xMousePos.value();
    if (xMousePosValue < 0.0)
    {
        xMousePosValue = 0.0;
    }
    std::size_t idxLower = static_cast<std::size_t>(std::floor(xMousePosValue / m_delta));
    idxLower = std::min(idxLower, m_yData.size() - 2);
    std::size_t idxUpper = idxLower + 1;


    // Calculate interpolated value
    double m = (m_yData[idxUpper] - m_yData[idxLower]) / m_delta;

    double xStart = idxLower * m_delta;
    double yPlotData =  m_yData[idxLower] + (xMousePosValue - xStart) * m;

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
