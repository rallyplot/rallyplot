#include <gtc/matrix_transform.hpp>
#include <qevent.h>
#include <tuple>
#include <QOpenGLWidget>
#include <QWidget>

#include "Camera.h"
#include "../structure/WindowViewportObject.h"
#include <QGuiApplication>
#include "../structure/LinkedSubplot.h"
#include "../structure/JointPlotData.h"


Camera::Camera(
    Configs& configs,
    LinkedSubplot& subplot,
    WindowViewportObject& windowViewport,
    JointPlotData& jointPlotData
)
    : m_configs(configs),
    m_sp(subplot),
    m_windowViewport(windowViewport),
    m_jointPlotData(jointPlotData)
{   
}


Camera::~Camera()
{
}


void Camera::setupView()
{
    initXView();
    setYLimitsFromView();
}

/* ----------------------------------------------------------------------------------------------------------
  Transformations
 ----------------------------------------------------------------------------------------------------------*/


void Camera::initXView()
{
/*
    Set the initial view width with the number of candles
    as set in the configs.
*/
    double delta = m_jointPlotData.getDelta();
    double numStartCandles = m_configs.m_numStartingDatapoints;

    double viewWidth =  numStartCandles * delta;

    if (m_configs.m_plotOptions.axisRight)
    {

        double maxX = m_jointPlotData.getDataMaxX();

        m_left = maxX - viewWidth;
        m_right = maxX;
    }
    else
    {
        double minX = (double)m_jointPlotData.getDataMinX();

        m_left = minX;
        m_right = minX + viewWidth;
    }
}


glm::mat4 Camera::getNDCMatrix()
/*
    Compute the transformation from view coordiantes to NDC.

    This includes an offset to set 0 as the left edge. This avoids rounding issues 
    for high number of candles. This needs to be matched in the vertex shader, and
    so `m_left` is passed via a uniform as an offset to be applied there.
*/
{
    if (m_sp.cameraSettings().yAxisLimitMode == YAxisMode::Pinned)
    {
        setYLimitsFromView();
    }

    glm::mat4 NDCMatrix = glm::ortho(
        0.0f, 
        (float)m_right - (float)m_left, 
        (float)m_bottom, 
        (float)m_top, 
        -1.0f, 
        1.0f
    );

    return NDCMatrix;
}


void Camera::setYLimitsFromView()
/*
    Set the y-limits to the max/data y-axis data
    value based on the current view.
*/
{
    std::tuple<double, double> minMax = m_jointPlotData.getMinMaxInViewRange(
        m_left,
        m_right
    );

    m_bottom = (double)std::floor(std::get<0>(minMax));
    m_top = (double)std::ceil(std::get<1>(minMax));
}


/* ----------------------------------------------------------------------------------------------------------
  Keyboard Zoom / Pan
 ----------------------------------------------------------------------------------------------------------*/
/*
    Process WASD keys used to control the camera.The cameraSpeed is
    scaled by the frame rate for cross-machine stability. Note here
    we are moving the camera position but keeping the camera direction
    fixed.

    This needs deltaTime because the speed is based on framerate
    (it's called once per render loop, wheras mouse callbacks are event-driven).
    I tried using callbacks for this but it did not work as well.
*/

void Camera::processKeyboardPressX(const std::unordered_map<int, bool>& keyStates)
{
    const double panValue = m_sp.cameraSettings().core.keyPanSpeed * 0.01;

    if (keyStates.at(Qt::Key_A))
        panX(-panValue);

    if (keyStates.at(Qt::Key_D))
        panX(panValue);

    const double zoomSpeed = m_sp.cameraSettings().core.keyZoomSpeed * 0.01;

    double centerX = (m_left + m_right) / 2.0;

    if (keyStates.at(Qt::Key_Z))
    {
        zoomX(1.0 + zoomSpeed, centerX, -1.0);
    }
    if (keyStates.at(Qt::Key_C))
    {
        zoomX(1.0 - zoomSpeed, centerX, -1.0);
    }
}


void Camera::processKeyboardPressY(const std::unordered_map<int, bool>& keyStates)
{
    const double panValue = m_sp.cameraSettings().core.keyPanSpeed * 0.01;

    if (keyStates.at(Qt::Key_W))
        panY(panValue, true);

    if (keyStates.at(Qt::Key_S))
        panY(-panValue, true);

    const double zoomSpeed = m_sp.cameraSettings().core.keyZoomSpeed * 0.01;

    double centerY = (m_bottom + m_top) / 2.0;

    if (keyStates.at(Qt::Key_Q))
    {
        zoomY(1.0 + zoomSpeed, centerY);
    }
    if (keyStates.at(Qt::Key_E))
    {
        zoomY(1.0 - zoomSpeed, centerY);
    }
}

/* ----------------------------------------------------------------------------------------------------------
    Mouse zoom / pan functions
----------------------------------------------------------------------------------------------------------*/
/*
    The change in mouse position is used to pan (left click) and zoom (right click)
    See getNDCMatrix() for how these are used.

    Everything is done in window space with margin removed, rather than viewport,
    because the viewport size is adjusted in the main loop, wheras the window size is
    always fixed (for one main loop iteration at least).

    When zooming or panning, the mouse should be locked to the application and
    hidden. It is hard to acheive this with QT API, instead platform-specific
    API are called in the connected slots to startZoomOrPan and endZoomOrPan.
*/


void Camera::storeViewClickPosition(double xMouseProportion, double yMouseProportion)
/*
    Internally store the last clicked position in view coordinates.
 */
{
    double viewXPos = getXValueUnderMouse(xMouseProportion);
    double viewYPos = getYValueUnderMouse(yMouseProportion);

    m_mouseState.viewClickPositionX = viewXPos;
    m_mouseState.viewClickPositionY = viewYPos;
    m_mouseState.clickXPosAsPercent = xMouseProportion;
}

double Camera::getYValueUnderMouse(double yMouseProportion)
/*
    Get y view coordinates from the click position
    (expressed as proportion of the view from the bottom)
 */
{
    return m_bottom + getViewHeight() * yMouseProportion;
}


double Camera::getXValueUnderMouse(double xMouseProportion)
{
    return  m_left + getViewWidth() * xMouseProportion;
}


int Camera::tickIndexUnderMouse(double xMouseProportion)
/*
    Get the index of the x data under the mouse position
    (expressed as proportion of the view from the left)
 */
{
    double delta = m_jointPlotData.getDelta();
    double viewXPos = m_left + getViewWidth() * xMouseProportion;
    int tickIndex = static_cast<int>(std::round(viewXPos / delta));

    // Handle edge rounding issues due to padding
    if (tickIndex < 0)
    {
        tickIndex = 0;
    }
    else if (tickIndex > m_jointPlotData.getNumDatapoints() - 1)
    {
        tickIndex = m_jointPlotData.getNumDatapoints() - 1;
    }

    return tickIndex;
}

std::string Camera::getXLabelUnderMouse(double xMouseProportion)
/*
    Similar to tickIndexUnderMouse, but get the
    tick label rather than the tick index.
 */
{
    int tickIndex = tickIndexUnderMouse(xMouseProportion);

    std::string xLabel = m_sp.sharedXData().getSingleFormattedLabel(tickIndex);

    return xLabel;
}


void Camera::leftMouseMove(double dx, double dy, bool changeY)
{
    double panSpeed = m_sp.cameraSettings().core.mousePanSpeed;

    // Scale by the window width to ensure similar feel when
    // panning at different window sizes. Not sure why but this
    // is not necessary for zooming. Scaling by width both times
    // is intentional as testing this works better than scaling separately
    double widthNoMargin = (double)m_windowViewport.getWidthNoMargin();
    double xPan = (dx * panSpeed) / widthNoMargin;
    double yPan = (dy * panSpeed * getAspectRatio()) / widthNoMargin;

    panX(-xPan);

    // do not apply aspect ratio, as incorporated
    // into the window correction above.
    if (changeY)
    {
        panY(yPan, false);
    }
}


void Camera::rightMouseMove(double dx, double dy, bool changeY)
{
    double mag = std::sqrt(dx*dx + dy*dy);

    double magThr = 50.0;

    if (mag > magThr)
    {
        double scalar = magThr / mag;
        dx *= scalar;
        dy *= scalar;
    }
    double zoomSpeed = m_sp.cameraSettings().core.mouseZoomSpeed;

    double subplotScaleX = 1.0; // / m_sp.windowViewport().subplotSizePercent().first;
    double subplotScaleY = 1.0; // / m_sp.windowViewport().subplotSizePercent().second + 1.0 / m_sp.m_yHeightProportion;

    double xScale = 1.0 + dx * zoomSpeed * subplotScaleX;
    double yScale = 1.0 - dy * zoomSpeed * subplotScaleY;

    zoomX(xScale, m_mouseState.viewClickPositionX, m_mouseState.clickXPosAsPercent);

    if (changeY)
    {
        zoomY(yScale, m_mouseState.viewClickPositionY);
    }
}


void Camera::wheelScrollZoom(double dy, std::optional<double> xMouseProportion, std::optional<double> yMouseProportion)
/*
    Zoom in or out with the mouse wheel.
*/
{
    double zoomSpeed = m_sp.cameraSettings().core.wheelSpeed;

    if (xMouseProportion.has_value())
    {
        double viewXPos = getXValueUnderMouse(xMouseProportion.value());
        double xValue = 1.0 + dy * zoomSpeed;
        zoomX(xValue, viewXPos, xMouseProportion.value());
    }

    if (yMouseProportion.has_value() && !(m_sp.cameraSettings().yAxisLimitMode == YAxisMode::Pinned))
    {
        double viewYPos = getYValueUnderMouse(yMouseProportion.value());
        double yValue = 1.0 + dy * zoomSpeed;
        zoomY(yValue, viewYPos);
    }
}


void Camera::fixZoomToEdge(bool on, bool rightMouseButtonPressed)
/*
    CTRL key explaination
    ---------------------
    We want CTRL press to switch the zoom mode. However, if the user starts zooming with CTRL pressed
    then releases, it will bug out during large zooms. We want to switch the zoom mode only after the 
    current zoom is finised. Therefore, if CTRL is released while the left mouse button is down, 
    zoomModeSwitched won't be turned off. In this case, we need to make sure zoomModeSwitched is turned 
    off at the end of the current zoom. Unfortunately there is no way around this.

    key : the Qt::Key_ pressed
    press : if true the action was a key press, otherwise key release

*/
{
    if (on)
    {
        m_ctrlPressed = true;
        m_zoomModeSwitched = true;
    }
    else
    {
        m_ctrlPressed = false;
        if (!rightMouseButtonPressed)
        {
            m_zoomModeSwitched = false;
        }
    }
}


void Camera::ensureZoomSwitchedModeOff()
{
    if (m_zoomModeSwitched && !m_ctrlPressed)
    {
       m_zoomModeSwitched = false;
    }
}


/* ----------------------------------------------------------------------------------------------------------
    Core zoom / pan functions
 ----------------------------------------------------------------------------------------------------------*/


void Camera::panX(double panValue)
/*
    This function pans the X axis, to the right when panValue is 
    positive and to the left when panValue is negative.

    Fundamentally this is a very simple procedure of adding an offset
    to the m_left and m_right boundary to move the view. However, cases
    in which we are at the x-axis limits must be handled, as well as if
    we are pinned to the primary x-axis.

    TODO
    ----
    In the case we are pinning to a primary x-axis, we dont want to pan past this 
    axis, not sure on the best approach here, could just return and do nothing.
*/
{
    if (panValue == 0)
         return;

    double viewWidth = getViewWidth();

    // If we are pining to an axis, don't allow panning past
    // that axis, and only extend the bound on the opposite side.
    if (m_sp.cameraSettings().core.lockMostRecentDate)
    {
        double& bound = (m_configs.m_plotOptions.axisRight) ? m_left : m_right;

        double offset = panValue * viewWidth;

        bound -= offset;

        if (bound < 0.0)
            bound = 0.0;

        return;
    }

    // Otherwise, just pan normally. Again we need to handle two cases.
    // If we are not at an edge boundary, then pan normally by 
    // adding the offset to the camera window. Otherwise, we are at a 
    // boundary which we do not want to go over. In this case, add how far
    // over that boundary back, so we are stuck at the boundary.

    double offset = panValue * viewWidth;  // scale pan to size of current view

    bool underMaxLimit;
    double offsetToMaxLimit;

    // TODO: own function with unpacking!
    double minX;
    double maxX;
    double padding;

    if (m_configs.m_xAxisLimitsOn)
    {
        minX = m_configs.m_xAxisLimits[0];
        maxX = m_configs.m_xAxisLimits[1];
        padding = m_jointPlotData.getDelta() / 2.0;
    }
    else
    {
        minX = 0;
        maxX = m_jointPlotData.getDataMaxX();
        padding = getPadding();
    }

    // Determine if we are over the edge of the left / right axis
    // (depending on whether we are panning right (pos) or left (neg)
    // and the associated offset to get back to the edge if we are.
    // Note it is necessary to do in this way, doing in the same way
    // as panY leads to panning at the border moving the pan window.
    if (utils_getSign(offset) == 1)
    {
        underMaxLimit = m_right < maxX + padding;
        offsetToMaxLimit = (m_right - maxX - padding);
    }
    else
    {
        underMaxLimit = m_left > minX - padding;
        offsetToMaxLimit = m_left - minX + padding;
    }

    // Apply offsets to pan the window
    if (underMaxLimit)
    {
        m_left += offset;
        m_right += offset;
    }
    else
    {
        offset = offsetToMaxLimit;
        m_left -= offset;
        m_right -= offset;
    }

}


double Camera::getPadding()
{
    return m_configs.m_xAxisPadding * m_jointPlotData.getDelta();
}


void Camera::zoomX(double zoomSpeed, double focusX, double xProportion)
/*
    Zoom the camera. If zooming with mouse, `focusX` is the mouse posiiton
    and xProportion is what proportion of the screen the click is e.g.
    close to the right edge will be near 1.

    If zooming with keyboard or mouse wheel, `focusX` should be
    set as the center of the view and xProportion set to `-1.0`,
    used to indicate we are not zooming with mouse.

    This function handles zooming around the `focusX` point as well
    as cases when we want to zoom pinned to the left or right axis.

*/
{
    double minX;
    double maxX;
    double padding;

    if (m_configs.m_xAxisLimitsOn)
    {
        minX = m_configs.m_xAxisLimits[0];
        maxX = m_configs.m_xAxisLimits[1];
        padding = m_jointPlotData.getDelta() / 2.0;
    }
    else
    {
        minX = 0;
        maxX = m_jointPlotData.getDataMaxX();
        padding = getPadding();
    }

    bool pinLeft = false;
    bool pinRight = false;

    // If we are locked to the main axis, we are always zooming
    // pinned to the left / right axis
    if (m_sp.cameraSettings().core.lockMostRecentDate)
    {
        if (!m_configs.m_plotOptions.axisRight)
        {
            pinLeft = true;
        }
        else
        {
            pinRight = true;
        }
    }

    // Otherwise, if we are using the modifier key to force zoom
    // pinned to the primary x-axis, then we want to pin.
    else if (
        (!m_sp.cameraSettings().core.fixZoomAtEdge && m_zoomModeSwitched)
        || (m_sp.cameraSettings().core.fixZoomAtEdge && !m_zoomModeSwitched)
        )
    {
        if (m_configs.m_plotOptions.axisRight)
        {
            pinLeft = false;
            pinRight = true;
        }
        else
        {
            pinLeft = true;
            pinRight = false;
        }
    }

    // Finally, if we are positioned flush against the left or right
    // axis, or we are clicking and the click is within 10% of the axis position,
    // then pin the zoom to the axis beause it feels more natural. Otherwise,
    // zoom around `focusX`.
    else
    {

        double xProportionCutoff = 0.01 * (maxX - minX);

        bool atLeftEdge = (m_left < minX + xProportionCutoff);
        bool atRightEdge = (m_right + xProportionCutoff > maxX);

        if (xProportion < 0)
        {
            pinRight = atRightEdge && !atLeftEdge;
            pinLeft = atLeftEdge && !atRightEdge;
        }
        else
        {
            bool clickRightEdge = xProportion > 1.0 - xProportionCutoff;
            bool clickLeftEdge = xProportion < xProportionCutoff;

            pinRight = atRightEdge && clickRightEdge;
            pinLeft = atLeftEdge && clickLeftEdge;
        }
    }

    // Don't allow zooming in too far
    if (zoomSpeed > 1.0 && m_right - m_left < m_jointPlotData.getDelta())
    {
        return;
    }

    // Apply the zoom.
    // If pinned, dont want the zoom speed to be dependent on 
    // click position, just extend in the opposite direction.
    // Otherwise, we are zooming around `focusX`.
    if (pinRight)
    {
        m_left = m_right - (m_right - m_left) / zoomSpeed;
    }
    else if (pinLeft)
    {
        m_right = m_left + (m_right - m_left) / zoomSpeed;
    }
    else
    {
        m_left = focusX - (focusX - m_left) / zoomSpeed;
        m_right = focusX + (m_right - focusX) / zoomSpeed;
    }

    // Finnally, ensure we cannot zoom over the axis limits
    // This is only really relevant zooming around `focusX`.
    // The check on m_left > 1.0 and corresponding m_right < 0.0
    // is to catch extreme degenerate state at the border when
    // using the trackpad
    if (m_left < minX - padding || m_left > maxX)
    {
        m_left = minX - padding;
    }
    if (m_right > maxX  + padding || m_right < minX)
    {
        m_right = maxX + padding;
    }
}


void Camera::panY(double panValue, bool applyAspectRatio)
/*
    Pan the y direction (scaled by view height). panValue can
    be positive or negative, which sets the pan direction.

    There are no limits of y-axis panning, which is why this
    function is simpler than the x-axis implementation.
*/
{
    double viewHeight = getViewHeight();
    double offset = panValue * viewHeight;

    if (applyAspectRatio)
    {
        offset *= getAspectRatio();
    }

    m_top += offset;
    m_bottom += offset;

    YAxisMode mode = m_sp.cameraSettings().yAxisLimitMode;
    if (mode == YAxisMode::FixedAuto || mode == YAxisMode::FixedUser)
    {
        double minLimit = m_sp.cameraSettings().yAxisLimits[0];
        double maxLimit = m_sp.cameraSettings().yAxisLimits[1];

        if (m_bottom < minLimit)
        {
            m_bottom = minLimit;
            m_top = minLimit + viewHeight;
        }
        else if (m_top > maxLimit)
        {
            m_top = maxLimit;
            m_bottom = maxLimit - viewHeight;
        }
    }
}


void Camera::zoomY(double zoomSpeed, double focusY)
/*
    Zoom the y-axis around the click position. The zoom width
    is not limited (unlike the x-axis) so the implementation is
    simpler.
*/
{
    if (m_sp.cameraSettings().yAxisLimitMode == YAxisMode::Pinned)
    {
        return;
    }

    m_top = focusY + ((m_top - focusY)) / zoomSpeed;
    m_bottom = focusY - ((focusY - m_bottom)) / zoomSpeed;

    YAxisMode mode = m_sp.cameraSettings().yAxisLimitMode;
    if (mode == YAxisMode::FixedAuto || mode == YAxisMode::FixedUser)
    {
        double minLimit = m_sp.cameraSettings().yAxisLimits[0];
        double maxLimit = m_sp.cameraSettings().yAxisLimits[1];

        if (m_bottom < minLimit)
        {
            m_bottom = minLimit;
        }
        if (m_top > maxLimit)
        {
            m_top = maxLimit;
        }
    }
}



/* ----------------------------------------------------------------------------------------------------------
  Helpers
 ----------------------------------------------------------------------------------------------------------*/


double Camera::getViewWidth() const
{
    return  (m_right - m_left);
}


double Camera::getViewHeight() const
{
    return m_top - m_bottom;
}


double Camera::getAspectRatio() const
{
    int windowWidth = m_windowViewport.getWindowWidth();
    int windowHeight = m_windowViewport.getWindowHeight();

    double plotAreaWidth = (double)windowWidth - m_configs.m_plotOptions.widthMarginSize * m_sp.pixelRatio();
    double plotAreaHeight = (double)windowHeight - m_configs.m_plotOptions.heightMarginSize * m_sp.pixelRatio();

    return plotAreaWidth / (plotAreaHeight * m_sp.m_yHeightProportion);
}


void Camera::resetYAxis()
{
    YAxisMode mode = m_sp.cameraSettings().yAxisLimitMode;
    if (mode == YAxisMode::FixedAuto || mode == YAxisMode::FixedUser)
    {
        m_bottom = m_sp.cameraSettings().yAxisLimits[0];
        m_top = m_sp.cameraSettings().yAxisLimits[1];
    }
    else // Pinned Axis
    {
        m_bottom = m_jointPlotData.getDataMinY();
        m_top = m_jointPlotData.getDataMaxY();
    }
}


void Camera::resetXAxis()
{
    double padding = getPadding();

    m_left = 0 - padding;
    m_right = m_jointPlotData.getDataMaxX() + padding;
}
