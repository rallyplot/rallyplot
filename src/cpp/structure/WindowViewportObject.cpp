#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include "WindowViewportObject.h"
#include "../structure/CentralOpenGlWidget.h"


WindowViewportObject::WindowViewportObject(Configs& configs, CentralOpenGlWidget& window, QOpenGLFunctions_3_3_Core& glFunctions)
    : m_configs(configs), m_window(window), m_gl(glFunctions)
{
    m_windowWidth = m_window.width();
    m_windowHeight = m_window.height();
}


std::pair<double, double> WindowViewportObject::subplotSizePercent()
{
    QSize mwSize = m_window.getMainwindowSize();
    return std::pair(m_window.width() / (double)mwSize.width(), m_window.height() / (double)mwSize.height());
}


WindowViewportObject::~WindowViewportObject()
{
}


double WindowViewportObject::pixelRatio()
{
    return m_window.devicePixelRatio();
}

/* -----------------------------------------------------------------------------------------------------------
    Public Functions
------------------------------------------------------------------------------------------------------------*/


void WindowViewportObject::setWindowWidth(int width)
{
    m_windowWidth = width;
}


void WindowViewportObject::setWindowHeight(int height)
{
    m_windowHeight = height;
}


double WindowViewportObject::yMarginAsProportion()
/*
    Return the y-margin (height allows in which to draw
    the x-axis axis and labels) as a percentage of the
    window height.
 */
{
    return m_configs.m_plotOptions.heightMarginSize / (double)m_window.height();
}


glm::mat4 WindowViewportObject::setForLinkedSubplotYAxis(double yStartProportion, double yHeightProportion)
/*

    Each subplot has its own y-axis. The x-axis is drawn by setting the viewport to full
    and then applying the transformation matrix to shift it into it's position, determined
    by xMargin. However, each subplots y dimension is determined by restricting the viewport
    when drawing. So we need to set the viewport full on the x-axis and restricted on the
    y-axis, then generate a viewport transform that shifts/scales on the x-axis only.

*/
{
    double xMargin = 0;
    double yMargin = m_configs.m_plotOptions.heightMarginSize * pixelRatio();

    double xWidth = m_windowWidth - xMargin;

    double yStart = yStartProportion * (double)(m_windowHeight - yMargin) + yMargin;
    double yHeight = yHeightProportion * ((double)m_windowHeight - yMargin);

    setRestrictedViewport(xMargin, yStart, xWidth, yHeight);

    return getFullViewportTransform(m_configs.m_plotOptions.widthMarginSize * pixelRatio(), 0.0);
}


void WindowViewportObject::setForLinkedSubplotPlot(double yStartProportion, double yHeightProportion)
/*
    Set a restricted viewport for drawing the actual plot.
    Unlike axis, we just restrict the viewport along the both
    x and y axis, then draw into it.
*/
{
    double xMargin = m_configs.m_plotOptions.widthMarginSize * pixelRatio();
    double yMargin = m_configs.m_plotOptions.heightMarginSize * pixelRatio();

    double xWidth = m_windowWidth - xMargin;

    double yStart = yStartProportion * (double)(m_windowHeight - yMargin) + yMargin;
    double yHeight = yHeightProportion * ((double)m_windowHeight - yMargin);

    setRestrictedViewport(xMargin, yStart, xWidth, yHeight);
}


glm::mat4 WindowViewportObject::setForSharedXAxis()
/*
    Set viewport for drawing the shared x-axis. In this case, we
    set the viewport to full and then generate a sclaing matrix
    to shift/scale the axis into position (defined by the x
    and y axis margins in the configs).
*/
{
    m_gl.glDisable(GL_SCISSOR_TEST);

    m_gl.glViewport(
        0,
        0,
        m_windowWidth,
        m_windowHeight
        );

    return getFullViewportTransform(m_configs.m_plotOptions.widthMarginSize * pixelRatio(), m_configs.m_plotOptions.heightMarginSize);
}


/* -----------------------------------------------------------------------------------------------------------
    Private Functions
------------------------------------------------------------------------------------------------------------*/


void WindowViewportObject::setRestrictedViewport(double xMargin, double yStart, double xWidth, double yHeight)
/*
    Lower-level function to restrict the viewport. Requires the start
    position and width of the displayed area.

    xMargin: The start position on which to restrict the viewport width.
             In practice this is always the x-axis margin, into which to
             draw the x-axis (and ticks)

    yStart: The start position of the y-axis, incorporating both the
            y-axis margin as well as offsets when there are multiple
            sub-plots.

    xWidth: Width of which to restrict the viewport.

    yHeight: Height of which to restrict the viewport.
*/
{

    int xStart;

    if (m_configs.m_plotOptions.axisRight)
    {
        xStart = 0;
    }
    else
    {
        xStart = xMargin;
    }

    m_gl.glViewport(
        xStart,
        yStart,
        m_windowWidth - xMargin,
        yHeight
        );

    m_gl.glScissor(
        xStart,
        yStart,
        m_windowWidth - xMargin,
        yHeight
        );
}


glm::mat4 WindowViewportObject::getFullViewportTransform(double xMargin, double yMargin)
/*
    Compute the transformation required to offset the x and y axis and axis labels
    such that they are drawn in the correct position when the viewport is full.
*/
{
    float width = m_windowWidth - xMargin;
    float height = m_windowHeight - yMargin;

    // Calculate how to translate the x and y coordinates:
    float offset_x = xMargin / m_windowWidth;
    float offset_y = yMargin / m_windowHeight;

    if (m_configs.m_plotOptions.axisRight)
    {
        offset_x *= -1;
    }

    // Calculate how to rescale the x and y coordinates:
    float scale_x = width / m_windowWidth;
    float scale_y = height / m_windowHeight;

    return glm::scale(  // rely on RVO
        glm::translate(glm::mat4(1), glm::vec3(offset_x, offset_y, 0)),
        glm::vec3(scale_x, scale_y, 1)
    );
}
