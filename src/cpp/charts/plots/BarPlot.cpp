#include "BarPlot.h"


BarPlot::BarPlot(
    Configs& configs,
    BackendBarSettings barSettings,
    QOpenGLFunctions_3_3_Core& glFunctions,
    const float* yPtr, std::size_t ySize
)
	: 
      m_barSettings(barSettings),
      m_gl(glFunctions),
      m_barProgram(
          "bar_vertex.shader",
          "bar_fragment.shader",
          glFunctions
      ),
      m_plotData(configs, barSettings.minValue, yPtr, ySize),
      m_barVAO(glFunctions),
      m_lineVAO(glFunctions)
{
    initializeAllBuffers();
    m_barProgram.setupAndBindProgram();
}


BarPlot::~BarPlot()
{
    m_gl.glDeleteBuffers(1, &m_barVBO);
    m_barVBO = 0;

    m_gl.glDeleteBuffers(1, &m_barBasisVBO);
    m_barBasisVBO = 0;
}


/* -----------------------------------------------------------
   Drawers
   ---------------------------------------------------------*/


void BarPlot::draw(glm::mat4& NDCMatrix, Camera& camera)
/*

*/
{
    m_gl.glEnable(GL_DEPTH_TEST);  // dont draw overlapping points (e.g. zoomed out)

    // Draw the body
    m_barProgram.bind();
    m_barProgram.setUniform1f("xDelta", (float)getPlotData().getDelta());
    m_barProgram.setUniformMatrix4fc("NDCMatrix", NDCMatrix);
    m_barProgram.setUniform1f("offset", (float)camera.getLeft());
    m_barProgram.setUniform1f("minValue", m_plotData.getMinValue());

    m_barProgram.setUniform4f("color", m_barSettings.color);
    m_barProgram.setUniform1f("widthRatio", m_barSettings.widthRatio);

    m_barVAO.bind();
    m_barProgram.setUniform1i("drawMode", 0);
    m_gl.glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, m_plotData.getNumDatapoints());

    // Draw lines behind the body to stop fade-into-white when zooming out.
    m_lineVAO.bind();
    m_barProgram.setUniform1i("drawMode", 1);
    m_gl.glDrawArraysInstanced(GL_LINES, 0, 2, m_plotData.getNumDatapoints());

    m_gl.glDisable(GL_DEPTH_TEST);

}


/* -----------------------------------------------------------
   Setup Buffers
------------------------------------------------------------*/


void BarPlot::initializeAllBuffers()
/*
*/
{
    m_barVAO.setup();

    // Setup the bar data buffer
    m_gl.glGenBuffers(1, &m_barVBO);
    m_gl.glBindBuffer(GL_ARRAY_BUFFER, m_barVBO);
    m_gl.glBufferData(GL_ARRAY_BUFFER, m_plotData.getYData().size() * sizeof(float), m_plotData.getYData().data(), GL_STATIC_DRAW);

    m_gl.glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)(0 * sizeof(float)));
    m_gl.glEnableVertexAttribArray(0);

	// Setup the body buffer and bind the associated instance VAO
    m_gl.glGenBuffers(1, &m_barBasisVBO);
    m_gl.glBindBuffer(GL_ARRAY_BUFFER, m_barBasisVBO);
    m_gl.glBufferData(GL_ARRAY_BUFFER, m_plotData.getBarBasis().size() * sizeof(float), m_plotData.getBarBasis().data(), GL_STATIC_DRAW);
    m_gl.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    m_gl.glEnableVertexAttribArray(1);

    m_gl.glVertexAttribDivisor(0, 1);

    // Setup the line buffer (shown behind the body)
    m_lineVAO.setup();
    m_gl.glBindBuffer(GL_ARRAY_BUFFER, m_barVBO);
    m_gl.glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)(0 * sizeof(float)));
    m_gl.glEnableVertexAttribArray(0);

    // Setup the body buffer and bind the associated instance VAO
    m_gl.glGenBuffers(1, &m_lineBasisVBO);
    m_gl.glBindBuffer(GL_ARRAY_BUFFER, m_lineBasisVBO);
    m_gl.glBufferData(GL_ARRAY_BUFFER, m_plotData.getLineBasis().size() * sizeof(float), m_plotData.getLineBasis().data(), GL_STATIC_DRAW);
    m_gl.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    m_gl.glEnableVertexAttribArray(1);

    m_gl.glVertexAttribDivisor(0, 1);
}

