#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include "../Camera.h"
#include "CandlestickData.h"
#include "../../opengl/VertexArrayObject.h"
#include "../../Configs.h"
#include "../shaders/Program.h"
#include "BasePlot.h"

class LinkedSubplot; // forward declaration

class CandlestickPlot : public FourValuePlot
/*
	Class to manage candlestick plotting.

	Contains VAO and Buffers for the instance array
	as well as basis shapes.

	There are 4 types of basis shape:

	body : the square that makes up the candle body
	candle : the full candlestick, including the gaps
	line : a vertical line. For candlestick no caps, this is
           between the low / high price. For no candle, this is still
		   displayed under the candle body. Without this line, the candles
		   start to dissapear when zoomed out far, and could not solve any other way.

	These are shift and scaled in the `candlestick_vertex.shader` 
	to their appropriate poisition on the plot.
*/
{

public:
    CandlestickPlot(
        Configs& configs,
        LinkedSubplot& subplot,
        BackendCandlestickSettings candlestickSettings,
        QOpenGLFunctions_3_3_Core& glFunctions,
        const float* openPtr, std::size_t openSize,
        const float* highPtr, std::size_t highSize,
        const float* lowPtr, std::size_t lowSize,
        const float* closePtr, std::size_t closeSize
    );
	~CandlestickPlot();

    const CandlestickData& getPlotData() const override { return m_plotData; };

	void cyclePlotType();

    void draw(glm::mat4& NDCMatrix, Camera& camera) override;

    CandlestickColor getPlotColor()const override {
        return  CandlestickColor{m_candlestickSettings.upColor, m_candlestickSettings.downColor};
    };

private:

    Configs& m_configs;
    LinkedSubplot& m_linkedSubplot;
    BackendCandlestickSettings m_candlestickSettings;
    QOpenGLFunctions_3_3_Core& m_gl;
    Program m_instanceProgram;
    Program m_lineProgram;
    Program m_oldPlotStyleProgram;
    CandlestickData m_plotData;

	void initializeAllBuffers();
	void rebindInstanceBuffer(bool setAttributeDivisor);

	unsigned int m_instanceVBO;
	unsigned int m_bodyBasisVBO;
	unsigned int m_candleBasisVBO;
	unsigned int m_lineBasisVBO;

    VertexArrayObject m_bodyVAO;
    VertexArrayObject m_candleVAO;
    VertexArrayObject m_lineVAO;
    VertexArrayObject m_linePlotVAO;

};
