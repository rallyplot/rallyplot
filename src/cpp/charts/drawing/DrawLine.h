#ifndef DRAWLINE_H
#define DRAWLINE_H

#include "../Camera.h"
#include "../../opengl/VertexArrayObject.h"
#include "../plots/BasePlotData.h"
#include "../shaders/Program.h"
#include <array>
#include <qopenglfunctions_3_3_core.h>


class LinkedSubplot;  // forward declaration


class DrawLine
{
public:
    DrawLine(
        QOpenGLFunctions_3_3_Core& glFunctions,
        float x,
        float y,
        LinkedSubplot& linkedSubplot,
        BackendDrawLineSettings drawLineSettings
    );

    void setup();
    void handleMouseMove(float x, float y);
    void draw(glm::mat4 NDCMatrix, Camera& camera);
    std::optional<UnderMouseData> getDataUnderMouse(double xValue, double yData, double yPadding, bool alwaysShow) const;

private:
    std::array<float, 4> m_drawPoints = {-0.5f, -0.5f, -0.5f, -0.5f}; // (x1, y1, x2, y2);  TODO: should use this for AxesObject!

    QOpenGLFunctions_3_3_Core& m_gl;
    VertexArrayObject m_vertexArray;
    Program m_program;
    LinkedSubplot& m_linkedSubplot;
    BackendDrawLineSettings m_drawLineSettings;

    unsigned int m_VBO;


};

#endif // DRAWLINE_H
