#include "ScatterplotData.h"


ScatterplotData::ScatterplotData(
    ScatterDateVector xData, const float* yPtr, std::size_t ySize, SharedXData& sharedXData
    ) : m_yData(yPtr, ySize), m_xData(m_heldXData.data(), m_heldXData.size())
{
    if (std::holds_alternative<StdPtrVector<int>>(xData))
    {
        // If a vector of int was passed, generate StrPtrVector from the ptr and size
        StdPtrVector<int> xVector = std::get<StdPtrVector<int>>(xData);

        m_xData = xVector;
    }
    else if (std::holds_alternative<StringVectorRef>(xData))
    {
        // Otherwise, if a vector of string was passed, convert to int and reference it in m_xData.
        m_heldXData = sharedXData.convertDateToIndex(
            std::get<StringVectorRef>(xData).get()
        );

        m_xData = StdPtrVector<int>(m_heldXData.data(), m_heldXData.size());
    }
    // TODO: this is exactly the same as above! should cast then just pass it
    else if (std::holds_alternative<TimepointVectorRef>(xData))
    {
        m_heldXData = sharedXData.convertDateToIndex(
            std::get<TimepointVectorRef>(xData).get()
            );

        m_xData = StdPtrVector<int>(m_heldXData.data(), m_heldXData.size());
    }
    m_hashedX = std::unordered_set<int>(m_xData.begin(), m_xData.end());
};


std::optional<UnderMouseData> ScatterplotData::getDataUnderMouse(
    int xIdx, double yData, double yPadding, bool alwaysShow, std::optional<double> xMousePos
) const
{

    // This is called on every repaint, and so we want to be very quick
    // in the (normal) case that the mouse is not over as scatterplot.
    // If it is, we take the hit and iterate over all points to find the value.
    // This is strange, why do this weird hashing rather than use std::unordered_map and
    // map hashed x to y directly? Because in sharedXData we need to iterative over the entirity
    // of the dataset to find min / max values. It will be slower to use map for this.
    // HOWEVER this is only done once (at plot set up), whereas this is every frame!
    // So it almost certainly makes sense to hash this to an unoreded map...
    // I guess another beenfit of this is we only copy the X rather than X and Y in case the vectors are passed by ref.
    if (m_hashedX.find(xIdx) == m_hashedX.end())
    {
        return std::nullopt;
    }

    std::optional<double> yPlotData = std::nullopt;
    for (int i = 0; i < m_yData.size(); i++)
    {
        if (xIdx == m_xData[i])
        {
            yPlotData = m_yData[i];
            break;
        }
    }

    if (!yPlotData.has_value())
    {
        return std::nullopt;
    }
    else
    {
        if (alwaysShow || (yPlotData.value() - yPadding < yData && yData < yPlotData.value() + yPadding))
        {
            return UnderMouseData {
                yPlotData.value()
            };
        }
        else
        {
            return std::nullopt;
        }
    }
}
