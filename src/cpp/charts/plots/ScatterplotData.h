#ifndef SCATTERPLOTDATA_H
#define SCATTERPLOTDATA_H

#include <unordered_set>
#include <vector>
#include "BasePlotData.h"
#include "../../include/Plotter.h"
#include "../../structure/SharedXData.h"
#include "../../include/UserVector.h"


class ScatterplotData : public OneValueData
{
public:
    ScatterplotData(ScatterDateVector xData, const float* yPtr, std::size_t ySize, SharedXData& sharedXData);

    const StdPtrVector<int>& getXData() const {return m_xData; }

    std::optional<UnderMouseData> getDataUnderMouse(
        int xIdx, double yData, double yPadding, bool alwaysShow, std::optional<double> xMousePos = std::nullopt
    ) const override;

    const StdPtrVector<float>& getYData() const override { return m_yData; };

    const StdPtrVector<float>& getMinVector() const override { return m_yData; };
    const StdPtrVector<float>& getMaxVector() const override { return m_yData; };

private:

    // Scatterplot x-data can be passed a vector of int or vector
    // of string that is converted to int. If a vector if int is passed,
    // we just hold a reference (StdPrVector). If a vector of int was generated
    // here from vector of string, then we hold the int vector and reference it in m_xData.
    const StdPtrVector<float> m_yData;
    std::vector<int> m_heldXData;
    StdPtrVector<int> m_xData;
    std::unordered_set<int> m_hashedX;
};

#endif
