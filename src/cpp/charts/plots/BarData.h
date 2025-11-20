#pragma once

#include <vector>
#include "../../Configs.h"
#include "BasePlotData.h"


class BarData : public OneValueData
/*
*/
{
public:
    BarData(Configs& configs, std::optional<float> minValue, const float* yPtr, std::size_t ySize);
    ~BarData();

    const std::vector<float>& getBarBasis() const { return m_barBasis; };
    const std::vector<float>& getLineBasis() const { return m_lineBasis; };

    const StdPtrVector<float>& getYData() const override { return m_yData; };

    std::optional<UnderMouseData> getDataUnderMouse(
        int xIdx, double yMousePos, double yPadding, bool alwaysShow, std::optional<double> xMousePos = std::nullopt
    ) const override;

    const float getMinValue() const { return m_minValue; };
    const StdPtrVector<float>& getMinVector() const override { return m_minVectorStrPtrVector; };
    const StdPtrVector<float>& getMaxVector() const override { return m_yData; };

private:

    Configs& m_configs;

    const StdPtrVector<float> m_yData;

	// Setup instance basis and vector pointers 
	// ----------------------------------------

    const std::vector<float> m_barBasis = {
    0.0f,  0.0f,   // bottom left
    0.0f,  1.0f,   // top left
    1.0f,  0.0f,   // bottom right
    1.0f,  1.0f    // top right
	};

    const std::vector<float> m_lineBasis = {
        0.0f, 0.0f,   // line bottom
        0.0f, 1.0f    // line top
    };

    float m_minValue;
    std::vector<float> m_minVector;
    StdPtrVector<float> m_minVectorStrPtrVector;
};




