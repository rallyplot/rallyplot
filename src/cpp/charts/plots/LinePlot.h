#ifndef LINEPLOT_H
#define LINEPLOT_H

#include "../Camera.h"
#include "../../Configs.h"
#include "LineData.h"
#include "../../opengl/VertexArrayObject.h"
#include "BasePlot.h"
#include "../shaders/Program.h"
#include <qopenglfunctions_3_3_core.h>
#include "../../structure/LinkedSubplot.h"


class LinePlot : public OneValuePlot
{

public:
    LinePlot(
        Configs& configs,
        BackendLineSettings lineSettings,
        LinkedSubplot& subplot,
        QOpenGLFunctions_3_3_Core& glFunctions,
        const float* yPtr, std::size_t ySize
    );
    ~LinePlot();

    void draw(glm::mat4& NDCMatrix, Camera& camera) override;

    const LineData& getPlotData() const override { return m_plotData; }
    PlotColor getPlotColor()const override { return  PlotColor{m_lineSettings.color}; };

private:

    BackendLineSettings m_lineSettings;
    LinkedSubplot& m_linkedSubplot;
    QOpenGLFunctions_3_3_Core& m_gl;
    Program m_lineProgram;
    Program m_oldPlotStyleProgram;
    LineData m_plotData;

    void initializeAllBuffers();

    unsigned int m_yDataVBO;
    VertexArrayObject m_yDataVAO;
};

#endif
