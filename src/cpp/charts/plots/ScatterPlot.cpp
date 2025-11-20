#include <iostream>
#include <qfileinfo.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "ScatterPlot.h"


ScatterPlot::ScatterPlot(
    BackendScatterSettings scatterSettings,
    SharedXData& sharedXData,
    QOpenGLFunctions_3_3_Core& glFunctions,
    ScatterDateVector xData,
    const float* yPtr, std::size_t ySize,
    LinkedSubplot& subplot
    ): m_scatterSettings(scatterSettings),
       m_gl(glFunctions),
       m_instanceProgram(
          "scatterplot_vertex.shader",
          "scatterplot_fragment.shader",
          glFunctions
       ),
    m_plotData(xData, yPtr, ySize, sharedXData),
    m_allDataVAO(glFunctions),
    m_linkedSubplot(subplot)
{
    initializeAllBuffers();
    setupTexture();
    m_instanceProgram.setupAndBindProgram();
}


ScatterPlot::~ScatterPlot()
{
    m_gl.glDeleteTextures(1, &m_circleTexture);
    m_gl.glDeleteTextures(1, &m_triangleUpTexture);
    m_gl.glDeleteTextures(1, &m_triangleDownTexture);
    m_gl.glDeleteTextures(1, &m_crossTexture);
}

void ScatterPlot::draw(glm::mat4& NDCMatrix, Camera& camera)
{
    m_gl.glEnable(GL_DEPTH_TEST);  // dont draw overlapping points (e.g. zoomed out)

    m_instanceProgram.bind();

    m_instanceProgram.setUniform1f("xDelta", (float)m_linkedSubplot.jointPlotData().getDelta());
    m_instanceProgram.setUniformMatrix4fc("NDCMatrix", NDCMatrix);
    m_instanceProgram.setUniform1f("offset", (float)camera.getLeft());
    m_instanceProgram.setUniform4f("color", m_scatterSettings.color);
    m_instanceProgram.setUniform1i("fixedSize", (int)m_scatterSettings.fixedSize);
    m_instanceProgram.setUniform1f("markerSizeFixed", m_scatterSettings.markerSizeFixed);
    m_instanceProgram.setUniform1f("markerSizeFree", m_scatterSettings.markerSizeFree);
    m_instanceProgram.setUniform1f("aspectRatio", camera.getAspectRatio());

    m_allDataVAO.bind();
    m_gl.glBindBuffer(GL_ARRAY_BUFFER, m_allDataVBO);
    m_gl.glBindBuffer(GL_ARRAY_BUFFER, m_quadInstanceVBO);

    m_gl.glActiveTexture(GL_TEXTURE1);

    ScatterShape shape = m_scatterSettings.shape;
    if (shape == ScatterShape::circle)
    {
        m_gl.glBindTexture(GL_TEXTURE_2D, m_circleTexture);
    }
    else if (shape == ScatterShape::triangleUp)
    {
        m_gl.glBindTexture(GL_TEXTURE_2D, m_triangleUpTexture);
    }
    else if (shape == ScatterShape::triangleDown)
    {
        m_gl.glBindTexture(GL_TEXTURE_2D, m_triangleDownTexture);
    }
    else if (shape == ScatterShape::cross)
    {
        m_gl.glBindTexture(GL_TEXTURE_2D, m_crossTexture);
    }
    else
    {
        std::cerr << "CRITICAL ERROR: `shape` " << utils_scatterShapeEnumToStr(shape) << " not recognised. This should be caught further up." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    m_instanceProgram.setUniform1i("shapeTexture", 1);
    m_gl.glDrawArraysInstanced(GL_TRIANGLES, 0, 6, m_plotData.getYData().size());

    m_gl.glDisable(GL_DEPTH_TEST);

}


void ScatterPlot::initializeAllBuffers()
{
    // Setup the instance buffer (shared between all VAO)
    m_allDataVAO.setup();
    m_gl.glGenBuffers(1, &m_allDataVBO);
    m_gl.glBindBuffer(GL_ARRAY_BUFFER, m_allDataVBO);

    const StdPtrVector<int>& xData = m_plotData.getXData();
    const StdPtrVector<float>& yData = m_plotData.getYData();

    // Annoying to have to copy here... I wonder if there is any way around it...
    std::vector<float> interleavedData(xData.size() * 2 + 1);

    double delta = m_linkedSubplot.jointPlotData().getDelta();

    for (int i = 0; i < xData.size(); i++)
    {
        interleavedData[i * 2] = (float)(delta * xData[i]);
        interleavedData[i * 2 + 1] = yData[i];
    }

    m_gl.glBufferData(
        GL_ARRAY_BUFFER,
        interleavedData.size() * sizeof(float),
        interleavedData.data(),
        GL_STATIC_DRAW
    );

    m_gl.glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    m_gl.glEnableVertexAttribArray(0);
    m_gl.glVertexAttribDivisor(0, 1);

    // The instance
    m_gl.glGenBuffers(1, &m_quadInstanceVBO);
    m_gl.glBindBuffer(GL_ARRAY_BUFFER, m_quadInstanceVBO);
    m_gl.glBufferData(
        GL_ARRAY_BUFFER,
        m_quadInstance.size() * sizeof(float),
        m_quadInstance.data(),
        GL_STATIC_DRAW
    );
    m_gl.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    m_gl.glEnableVertexAttribArray(1);
}


void ScatterPlot::setupTexture()
{
    m_gl.glGenTextures(1, &m_circleTexture);
    m_gl.glBindTexture(GL_TEXTURE_2D, m_circleTexture);
    loadTexture(":/scatterTexture/charts/plots/textures/circle.png");

    m_gl.glGenTextures(1, &m_triangleUpTexture);
    m_gl.glBindTexture(GL_TEXTURE_2D, m_triangleUpTexture);
    loadTexture(":/scatterTexture/charts/plots/textures/triangle_up.png");

    m_gl.glGenTextures(1, &m_triangleDownTexture);
    m_gl.glBindTexture(GL_TEXTURE_2D, m_triangleDownTexture);
    loadTexture(":/scatterTexture/charts/plots/textures/triangle_down.png");

    m_gl.glGenTextures(1, &m_crossTexture);
    m_gl.glBindTexture(GL_TEXTURE_2D, m_crossTexture);
    loadTexture(":/scatterTexture/charts/plots/textures/cross.png");

    // this must be turned off for Qt QPainter to work
    // properly over OpenGl widget.
    m_gl.glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}


void ScatterPlot::loadTexture(std::string fileName)
{
    m_gl.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    QFile file(QString::fromStdString(fileName));

    if (!file.open(QIODevice::ReadOnly))
    {
        std::cerr << "CRITICAL ERROR: Failed to open resource file: " << fileName << std::endl;
        std::exit(EXIT_FAILURE);
    }

    QByteArray imageData = file.readAll();
    file.close();

    stbi_set_flip_vertically_on_load(true);

    int width, height, numChannels;
    unsigned char* data = stbi_load_from_memory(
        reinterpret_cast<const unsigned char*>(imageData.constData()),
        imageData.size(),
        &width,
        &height,
        &numChannels,
        4
    );

    if (!data)
    {
        std::cerr << "CRITICAL ERROR: Failed to load png texture." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    m_gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);

    m_gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
