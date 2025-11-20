#pragma once

#include "../../include/UserVector.h"
#include <vector>

#include "../../Configs.h"
#include "BasePlotData.h"


class CandlestickData : public FourValueData
/*
	Class for managing the array vector that 
	contains the candlestick body and caps.

	More details on the plotting can be found in `CandleStickPlot.h` and
    `candlestick_vertex.shader`. Breifly, data held here are the
    loaded candle data (xPos, Open, High, Low, Close) and a set
	of basis shapes used for instancing.

	In the shader, the basis shapes are scaled by the Open, Close etc.
	such that they appear in the correct view coordinates 
	(i.e. windowed world coordinates). 
*/
{
public:
    CandlestickData(
        Configs& configs,
        const float* openPtr, std::size_t openSize,
        const float* highPtr, std::size_t highSize,
        const float* lowPtr, std::size_t lowSize,
        const float* closePtr, std::size_t closeSize
    );
	~CandlestickData();

    const std::vector<float>& getBodyBasis() const { return m_bodyBasis; };
    const std::vector<float>& getCandleBasis() const { return m_candleBasis;  };
    const std::vector<float>& getLineBasis() const { return m_lineBasis; };

    const StdPtrVector<float>& getMinVector() const override { return m_low; };
    const StdPtrVector<float>& getMaxVector() const override { return m_high; };

    std::optional<UnderMouseData> getDataUnderMouse(
        int xIdx, double yMousePos, double yPadding, bool alwaysShow, std::optional<double> xMousePos = std::nullopt
    ) const override;

    const StdPtrVector<float> m_open;
    const StdPtrVector<float> m_high;
    const StdPtrVector<float> m_low;
    const StdPtrVector<float> m_close;

private:

    Configs& m_configs;

	// Setup instance basis and vector pointers 
	// ----------------------------------------

    const std::vector<float> m_bodyBasis = {
	0.0f,  0.0f,   // top left
	0.0f, -1.0f,   // bottom left
	1.0f,  0.0f,   // top right
	1.0f, -1.0f    // bottom right
	};

    const std::vector<float> m_candleBasis = {
		-0.5f,  0.0f,      // top cap
		 0.5f,  0.0f,

		-0.5f, -1.0f,      // bottom cap
		 0.5f, -1.0f,

		 0.0f,  0.0f,      // connecting line
		 0.0f, -1.0f
    };

    const std::vector<float> m_lineBasis = {
        0.0f,  0.0f,    // line top
		0.0f, -1.0f    // line bottom
	};
};
