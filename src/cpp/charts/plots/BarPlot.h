#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include "../Camera.h"
#include "../../opengl/VertexArrayObject.h"
#include "../../Configs.h"
#include "BarData.h"
#include "../shaders/Program.h"
#include "BasePlot.h"


class BarPlot : public OneValuePlot
/*
*/
{

public:
    BarPlot(
        Configs& configs,
        BackendBarSettings barSettings,
        QOpenGLFunctions_3_3_Core& glFunctions,
        const float* yPtr, std::size_t ySize
    );
    ~BarPlot();

    const BarData& getPlotData() const override { return m_plotData; };

    PlotColor getPlotColor()const override { return  PlotColor{m_barSettings.color}; };

    void draw(glm::mat4& NDCMatrix, Camera& camera) override;

private:

    BackendBarSettings m_barSettings;
    QOpenGLFunctions_3_3_Core& m_gl;
    Program m_barProgram;
    BarData m_plotData;

	void initializeAllBuffers();
	void rebindInstanceBuffer(bool setAttributeDivisor);

    unsigned int m_barVBO;
    unsigned int m_barBasisVBO;
    unsigned int m_lineBasisVBO;

    VertexArrayObject m_barVAO;
    VertexArrayObject m_lineVAO;

};
