#ifndef LINEDATA_H
#define LINEDATA_H

#include "../../Configs.h"
#include "BasePlotData.h"


class LineData : public OneValueData
{
public:
    LineData(Configs& configs, const float* yPtr, std::size_t ySize);

    const StdPtrVector<float>& getYData() const override { return m_yData; };

    std::optional<UnderMouseData> getDataUnderMouse(
        int _, double yData, double yPadding, bool alwaysShow, std::optional<double> xMousePos
    ) const override;

    const StdPtrVector<float>& getMinVector() const override { return m_yData; };
    const StdPtrVector<float>& getMaxVector() const override { return m_yData; };

private:
    Configs& m_configs;

    const StdPtrVector<float> m_yData;

};

#endif
