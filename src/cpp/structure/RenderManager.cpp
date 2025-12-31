#include "RenderManager.h"


RenderManager::RenderManager(CentralOpenGlWidget& window, Configs& configs, QOpenGLFunctions_3_3_Core& glFunctions)
    : m_configs(configs),
    m_gl(glFunctions),
    m_windowViewport(configs, window, glFunctions),
    m_sharedXData{}
{
     Q_INIT_RESOURCE(resources);

    // Initialise the first subplot
    m_linkedSubplots.reserve(1);
    std::unique_ptr<LinkedSubplot> subplot = std::make_unique<LinkedSubplot>(
        *this,
        m_configs,
        m_configs.m_defaultCameraSettings,
        m_configs.m_defaultXAxisSettings,
        m_configs.m_defaultYAxisSettings,
        m_sharedXData,
        m_gl,
        m_windowViewport, 0.0, 1.0
    );
    m_linkedSubplots.emplace_back(std::move(subplot));

    m_gl.glDisable(GL_CULL_FACE);
    m_gl.glDisable(GL_DEPTH_TEST);

};


RenderManager::~RenderManager()
{
}


void RenderManager::paint()
/*
    Render loop for this subplot.
*/
{
    // Clear the screen

    setBackgroundColor(m_configs);
    m_gl.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_linkedSubplots[0]->jointPlotData().isEmpty())
    {
        return;
    }

    // Draw the X axis (first, so candlestick are in front of gridlines)

    glm::mat4 viewportTransform = m_linkedSubplots[0]->windowViewport().setForSharedXAxis();

    m_linkedSubplots[0]->axesObject().drawAxes(viewportTransform, "x");
    m_linkedSubplots[0]->axesObject().drawXAxesAndTicks(viewportTransform);

    // Draw all subplots

    for (const std::unique_ptr<LinkedSubplot>& subplot : m_linkedSubplots)
    {
        subplot->draw();
    }

    m_windowViewport.setForSharedXAxis();  // must do this here (or move to start of loop)
}


void RenderManager::addLinkedSubplot(double heightAsProportion)
/*
    Add a subplot to the plot.
 */
{
    // For all new linked subplots, simply subtract the space from the first plot. This leads to
    // most intuitive scaling, and it is simple to rescale when desired.
    m_linkedSubplots[0]->m_yHeightProportion = m_linkedSubplots[0]->m_yHeightProportion - heightAsProportion;

    // Next initialise and store the subplot, using the most recent subplot camera
    // settings (in case the user has edited them)
    BackendCameraSettings lastCameraSettings = m_linkedSubplots[m_linkedSubplots.size() - 1]->cameraSettings();
    BackendAxisSettings lastXAxisSettings = m_linkedSubplots[m_linkedSubplots.size() - 1]->xAxisSettings();
    BackendAxisSettings lastYAxisSettings = m_linkedSubplots[m_linkedSubplots.size() - 1]->yAxisSettings();

    std::unique_ptr<LinkedSubplot> subplot = std::make_unique<LinkedSubplot>(
        *this,
        m_configs,
        lastCameraSettings,
        lastXAxisSettings,
        lastYAxisSettings,
        m_sharedXData,
        m_gl,
        m_windowViewport,
        0.0,
        heightAsProportion
    );

    m_linkedSubplots.emplace_back(
        std::move(subplot)
    );

    // Finally, shift the position of all plots to accommodate the new subplot
    double cumuHeight = 0.0;

    for (int i = m_linkedSubplots.size() - 1; i >= 0; i--)
    {
        double height = m_linkedSubplots[i]->m_yHeightProportion;
        m_linkedSubplots[i]->m_yStartProportion = cumuHeight;
        cumuHeight += height;
    }

}

void RenderManager::resizeLinkedSubplots(std::vector<double> yHeights)
{
    double cumuHeight = 0.0;
    for (int i = m_linkedSubplots.size() - 1; i >= 0; i--)
    {
        m_linkedSubplots[i]->m_yHeightProportion = yHeights[i];
        m_linkedSubplots[i]->m_yStartProportion = cumuHeight;
        cumuHeight += yHeights[i];
    }

}


void RenderManager::setBackgroundColor(Configs& configs)
{
    m_gl.glClearColor(
        m_configs.m_backgroundColor.r,
        m_configs.m_backgroundColor.g,
        m_configs.m_backgroundColor.b,
        m_configs.m_backgroundColor.a
    );
};


void RenderManager::updateWindowSize(int width, int height)
{
    for (const std::unique_ptr<LinkedSubplot>& subplot : m_linkedSubplots)
    {
        subplot->windowViewport().setWindowWidth(width);
        subplot->windowViewport().setWindowHeight(height);
    }
}
