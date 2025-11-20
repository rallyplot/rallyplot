#include "DrawLine.h"
#include "../../structure/LinkedSubplot.h"

DrawLine::DrawLine(QOpenGLFunctions_3_3_Core& glFunctions, float x, float y, LinkedSubplot& linkedSubplot, BackendDrawLineSettings drawLinesettings)
    : m_gl(glFunctions),
    m_vertexArray(m_gl),
    m_program(
        "simple_line_vertex.shader",
        "simple_line_fragment.shader",
        "simple_line_geomtry.shader",  // TODO: RENAME FOR ALL!
        m_gl
        ),
    m_linkedSubplot(linkedSubplot),
    m_drawLineSettings(drawLinesettings)
{
    m_drawPoints = {x, y, x, y};


    // TODO: just pass as uniforms!!!!!
    m_vertexArray.setup();

    m_gl.glGenBuffers(1, &m_VBO);
    m_gl.glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    m_gl.glBufferData(GL_ARRAY_BUFFER, sizeof(m_drawPoints), m_drawPoints.data(), GL_DYNAMIC_DRAW);

    m_gl.glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    m_gl.glEnableVertexAttribArray(0);

    m_vertexArray.unBind();

    m_program.setupAndBindProgram();
};


void DrawLine::handleMouseMove(float x, float y)  // TODO: x should already be in delta
{
    m_drawPoints = { m_drawPoints[0], m_drawPoints[1], x, y};

    m_gl.glBindBuffer(GL_ARRAY_BUFFER, m_VBO);  // HANDLE DUPL!
    m_gl.glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(m_drawPoints), m_drawPoints.data());
    m_gl.glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void DrawLine::draw(glm::mat4 NDCMatrix, Camera& camera)
{

    m_gl.glEnable(GL_BLEND);

    m_program.bind();
    m_program.setUniformMatrix4fc("NDCMatrix", NDCMatrix);
    m_program.setUniform1f("offset", (float)camera.getLeft());

    m_program.setUniform1f("aspectRatio", camera.getAspectRatio());
    m_program.setUniform4f("color", m_drawLineSettings.color);
    m_program.setUniform1f("width", m_drawLineSettings.linewidth / 100.0);
    m_program.setUniform1f("yHeightProportion", m_linkedSubplot.m_yHeightProportion);
    m_program.setUniform1f("subplotHeightProportion", m_linkedSubplot.windowViewport().subplotSizePercent().second);
    m_vertexArray.bind();

    m_gl.glDrawArrays(GL_LINE_STRIP, 0, 2);

    m_gl.glDisable(GL_BLEND);
}


std::optional<UnderMouseData> DrawLine::getDataUnderMouse(double xValue, double yData, double yPadding,  bool alwaysShow) const
{
    double startX =  m_drawPoints[0];
    double endX =  m_drawPoints[2];

    if (!(startX < xValue && xValue < endX))
    {
        return std::nullopt;
    }

    double startY = m_drawPoints[1];
    double endY = m_drawPoints[3];
    double m = (endY - startY) / (endX - startX);  // TODO: should incorporate width! will need to move all of this to the plot level...

    double yPlotData = startY + (xValue - startX) * m;

    if (alwaysShow || (yPlotData - yPadding < yData && yData < yPlotData + yPadding))
    {
        return UnderMouseData {
            yPlotData
        };
    }
    else
    {
        return std::nullopt;
    }
}

