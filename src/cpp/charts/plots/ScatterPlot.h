#ifndef SCATTERPLOT_H
#define SCATTERPLOT_H

#include "../../Configs.h"
#include "../../opengl/VertexArrayObject.h"
#include "BasePlot.h"
#include "ScatterplotData.h"
#include "../shaders/Program.h"
#include <qopenglfunctions_3_3_core.h>
#include "../../structure/LinkedSubplot.h"
#include "../../structure/SharedXData.h"
#include "../../include/Plotter.h"


class ScatterPlot : public OneValuePlot
{
public:
    ScatterPlot(
        BackendScatterSettings scatterSettings,
        SharedXData& sharedXData,
        QOpenGLFunctions_3_3_Core& glFunctions,
        ScatterDateVector xData,
        const float* yPtr, std::size_t ySize,
        LinkedSubplot& renderManager  // TEMP
    );
    ~ScatterPlot();

    void draw(glm::mat4& NDCMatrix, Camera& camera) override;

    const ScatterplotData& getPlotData() const override { return m_plotData; }

    void setupTexture();

    PlotColor getPlotColor()const override { return  PlotColor{m_scatterSettings.color}; };


private:

    BackendScatterSettings m_scatterSettings;
    QOpenGLFunctions_3_3_Core& m_gl;
    Program m_instanceProgram;
    ScatterplotData m_plotData;

    void initializeAllBuffers();
    void loadTexture(std::string fileName);

    unsigned int m_allDataVBO;
    VertexArrayObject m_allDataVAO;

    LinkedSubplot& m_linkedSubplot;

    unsigned int m_circleTexture = 0;
    unsigned int m_triangleUpTexture = 0;
    unsigned int m_triangleDownTexture = 0;
    unsigned int m_crossTexture = 0;

    unsigned int m_quadInstanceVBO;

    const std::vector<float> m_quadInstance{
        -1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f, -1.0f,

        -1.0f,  1.0f,
         1.0f,  1.0f,
         1.0f, -1.0f
    };
};

#endif
