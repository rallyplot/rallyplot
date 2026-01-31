#include <cassert>
#include "CandlestickData.h"


CandlestickData::CandlestickData(
    Configs& configs,
    const float* openPtr, std::size_t openSize,
    const float* highPtr, std::size_t highSize,
    const float* lowPtr, std::size_t lowSize,
    const float* closePtr, std::size_t closeSize
)
    : m_open(openPtr, openSize),
    m_high(highPtr, highSize),
    m_low(lowPtr, lowSize),
    m_close(closePtr, closeSize),
    m_configs(configs)
/*
    Organisation in memory is (xPos, Open, Close, Low, High) and
    sizing is performed in the `candlestick_vertex.shader`.
*/
{
    m_numDataPoints =  openSize;
    m_delta = 1.0 / m_numDataPoints;
}


CandlestickData::~CandlestickData()
{
}


std::optional<UnderMouseData> CandlestickData::getDataUnderMouse(
    int xIdx,
    double yMousePos,
    double yPadding,
    bool alwaysShow,
    std::optional<double> xMousePos
) const
{
    float low = m_low[xIdx];
    float high = m_high[xIdx];

    if (alwaysShow || (low - yPadding < yMousePos && yMousePos < high + yPadding))
    {
        return UnderMouseData {
            CandleInfo { m_open[xIdx], high, low, m_close[xIdx]}
        };
    }
    else
    {
        return std::nullopt;
    }
}
