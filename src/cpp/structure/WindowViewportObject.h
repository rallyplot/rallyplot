#pragma once

#include "../Configs.h"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <qopenglfunctions_3_3_core.h>


 // forward definitions
class RenderManager;
class CentralOpenGlWidget;

class WindowViewportObject
/*
    Class to manage viewport calculations. The viewport
    is changed in order to draw the full plot vs. axes
    and axes labels.

    The key public functions are:

    glm::mat4 setForSharedXAxis :
        Set the viewport for the shared X-axis, by removing
        all margins. Return a transform to draw the x-axis
        and x-axis labels (shared between all plots) in the
        correct position.

    void setForLinkedSubplotPlot :
        Set the viewport for a given subplot. This simply restricts
        the viewport and the plot is draw within it.

    glm::mat4 setForLinkedSubplotYAxis :
        Set the viewport for drawing the y-axis. This restricts the
        viewport along the y-axis direction, and sets it full along
        the shared x-axis direction. The returned transformation
        ensures the y-axis and y-axis labels are drawn in the correct
        position according to the x-axis.

    The margin size within which the axis and axis labels
    are drawn are set in configs.
*/
{

public:

    WindowViewportObject(Configs& configs, CentralOpenGlWidget& window, QOpenGLFunctions_3_3_Core& glFunctions);
	~WindowViewportObject();

    WindowViewportObject(const WindowViewportObject&) = delete;
    WindowViewportObject& operator=(const WindowViewportObject&) = delete;
    WindowViewportObject(WindowViewportObject&&) = delete;
    WindowViewportObject& operator=(WindowViewportObject&&) = delete;

    void setWindowWidth(int width);
    void setWindowHeight(int height);
    double yMarginAsProportion();

    glm::mat4 setForSharedXAxis();

    void setForLinkedSubplotPlot(double yStartProportion, double yHeightProportion);
    std::pair<double, double> subplotSizePercent();

    glm::mat4 setForLinkedSubplotYAxis(double yStartProportion, double yHeightProportion);

    int getWindowWidth() const { return m_windowWidth; };
	int getWindowHeight() const { return m_windowHeight; };

    int getWidthNoMargin() const { return m_windowWidth - m_configs.m_plotOptions.widthMarginSize; };
    int getHeightNoMargin() const { return m_windowHeight - m_configs.m_plotOptions.heightMarginSize; };

    double pixelRatio();

private:

    Configs& m_configs;
    CentralOpenGlWidget& m_window;
    QOpenGLFunctions_3_3_Core& m_gl;

    void setRestrictedViewport(double xMargin, double yMargin, double xWidth, double xHeight);

    glm::mat4 getFullViewportTransform(double xMargin, double yMargin);

	int m_windowWidth;
    int m_windowHeight;
};
