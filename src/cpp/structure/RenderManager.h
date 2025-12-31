#pragma once

#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>

#include "WindowViewportObject.h"
#include "../charts/Camera.h"
#include "../Configs.h"
#include "LinkedSubplot.h"
#include "SharedXData.h"


class RenderManager
/*
    Class to organise the draw loop and coordination of all plot classes,
    axes class, axis tick / tick label classes.

    Implements the render loop.
*/
{

public:

    RenderManager(CentralOpenGlWidget &window, Configs &configs,
                  QOpenGLFunctions_3_3_Core& glFunctions);

    ~RenderManager();

    RenderManager(const RenderManager&) = delete;
    RenderManager& operator=(const RenderManager&) = delete;
    RenderManager(RenderManager&&) = delete;
    RenderManager& operator=(RenderManager&&) = delete;

    void paint();

    void addLinkedSubplot(double heightAsProportion);
    void resizeLinkedSubplots(std::vector<double> yHeights);

    void updateWindowSize(int width, int height);

    Configs& configs() { return m_configs; };

    Configs& m_configs;
    QOpenGLFunctions_3_3_Core& m_gl;
    WindowViewportObject m_windowViewport;
    std::vector<std::unique_ptr<LinkedSubplot>> m_linkedSubplots;
    SharedXData m_sharedXData;

private:

    void setBackgroundColor(Configs& configs);
};
