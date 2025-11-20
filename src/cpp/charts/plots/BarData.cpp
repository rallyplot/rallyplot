#include <cassert>
#include "BarData.h"


BarData::BarData(Configs& configs, std::optional<float> minValue, const float* yPtr, std::size_t ySize)
    : m_configs(configs), m_yData(yPtr, ySize)
{
    m_numDataPoints =  m_yData.size();
    m_delta = 1.0 / m_numDataPoints;

    float minValueFinal;
    if (!minValue.has_value())
    {
        // auto [minIt, maxIt] = std::minmax_element(m_yData.data(), m_yData.data() + m_yData.size());
        // minValueFinal = *minIt - 0.01f * (*maxIt - *minIt);
        minValueFinal = 0.0f;
    }
    else
    {
        minValueFinal = minValue.value();
    }

    // TODO: This is wasteful, but is kept for now as we need to return a 'minimum vector'
    // for min / max view computation. It would be better to return a float and then
    // hanle this in JointPlotData.cpp
    m_minValue = minValueFinal;
    m_minVector = std::vector<float>(ySize, minValueFinal);
    m_minVectorStrPtrVector = StdPtrVector<float>(m_minVector.data(), m_minVector.size());
}


BarData::~BarData()
{
}


std::optional<UnderMouseData> BarData::getDataUnderMouse(
    int xIdx, double yMousePos, double yPadding, bool alwaysShow, std::optional<double> xMousePos
) const
{

    double yPlotData = m_yData[xIdx];

    bool isUnderMouse = false;
    if (alwaysShow || (0 - yPadding < yMousePos && yMousePos < yPlotData + yPadding))
    {
        return UnderMouseData {
            yPlotData, isUnderMouse
        };
    }
    else
    {
        return std::nullopt;
    }

}
