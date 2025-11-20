#include "LinePlot.h"


LinePlot::LinePlot(
    Configs& configs,
    BackendLineSettings lineSettings,
    LinkedSubplot& subplot,
    QOpenGLFunctions_3_3_Core& glFunctions,
    const float* yPtr, std::size_t ySize
)
    : m_lineSettings(lineSettings),
    m_linkedSubplot(subplot),
    m_gl(glFunctions),
    m_lineProgram(
          "line_vertex.shader",
          "line_fragment.shader",
          "line_geometry.shader",
          glFunctions
          ),
    m_oldPlotStyleProgram("line_vertex.shader", "line_fragment.shader", glFunctions),
    m_plotData(configs, yPtr, ySize),
    m_yDataVAO(glFunctions)
{
    initializeAllBuffers();
    m_lineProgram.setupAndBindProgram();
    m_oldPlotStyleProgram.setupAndBindProgram();
}


LinePlot::~LinePlot()
{
    m_gl.glDeleteBuffers(1, &m_yDataVBO);
    m_yDataVBO = 0;
}


void LinePlot::draw(glm::mat4& NDCMatrix, Camera& camera)
{

    m_gl.glEnable(GL_DEPTH_TEST);  // dont draw overlapping points (e.g. zoomed out)

    if (!m_lineSettings.basicLine)
    {
        m_lineProgram.bind();
        m_lineProgram.setUniform1f("xDelta", (float)getPlotData().getDelta());

        m_lineProgram.setUniformMatrix4fc("NDCMatrix", NDCMatrix);
        m_lineProgram.setUniform1f("offset", (float)camera.getLeft());
        m_lineProgram.setUniform1f("aspectRatio", camera.getAspectRatio());
        m_lineProgram.setUniform4f("color", m_lineSettings.color);
        m_lineProgram.setUniform1f("width", m_lineSettings.width / 100.0);
        m_lineProgram.setUniform1f("miterLimit", m_lineSettings.miterLimit);
        m_lineProgram.setUniform1f("yHeightProportion", m_linkedSubplot.m_yHeightProportion);
        m_lineProgram.setUniform1i("numVertices", m_plotData.getYData().size());
        m_lineProgram.setUniform1f("subplotHeightProportion", m_linkedSubplot.windowViewport().subplotSizePercent().second); // TODO: this naming is super confusing, this is the high level subplot

        m_yDataVAO.bind();

        m_gl.glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, m_plotData.getYData().size());
    }
    else
    {
        m_oldPlotStyleProgram.bind();
        m_oldPlotStyleProgram.setUniform1f("xDelta", (float)getPlotData().getDelta());

        m_oldPlotStyleProgram.setUniformMatrix4fc("NDCMatrix", NDCMatrix);
        m_oldPlotStyleProgram.setUniform1f("offset", (float)camera.getLeft());
        m_oldPlotStyleProgram.setUniform4f("color", m_lineSettings.color);
        m_yDataVAO.bind();

        m_gl.glDrawArrays(GL_LINE_STRIP, 0, m_plotData.getYData().size());
    }
    m_gl.glDisable(GL_DEPTH_TEST);
}


void LinePlot::initializeAllBuffers()
{
    // Setup the instance buffer (shared between all VAO)
    m_yDataVAO.setup();
    m_gl.glGenBuffers(1, &m_yDataVBO);
    m_gl.glBindBuffer(GL_ARRAY_BUFFER, m_yDataVBO);

    m_gl.glBufferData(
        GL_ARRAY_BUFFER,
        m_plotData.getYData().size() * sizeof(float),
        m_plotData.getYData().data(),
        GL_STATIC_DRAW
    );

    m_gl.glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);
    m_gl.glEnableVertexAttribArray(0);
}

