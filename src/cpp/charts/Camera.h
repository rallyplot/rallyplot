# pragma once

#include <QWidget>
#include <QCursor>
#include <glm.hpp>
#include <QOpenGLFunctions_3_3_Core>
#include <unordered_map>
#include "../structure/WindowViewportObject.h"


// forward declarations
class LinkedSubplot;
class JointPlotData;
class CentralOpenGlWidget;

// Mouse state during zooming and panning.
// See mouse_button_callback() for more details
struct MouseState
{
    double clickXPosAsPercent;
    double viewClickPositionX;
    double viewClickPositionY;
    double viewDistMovedX;
	double viewDistMovedY;
};


class Camera
/*
    Controls the camera position that windows the data.

    Terminology:
    - 'World' this is the data coordinates (x-axis is candlestick index, y-axis is the price)
    - 'View' is in the same coordinate system, but restricted to the current view of the data
    - 'viewport' is whether the viewport is across the entire window or a subsection (e.g. for axis)

    List of features
    ----------------

    - right mouse pan
    - left mouse zoom
    - R to reset window
    - (optional) x-axis padding value (configs)
    - set custom y axis limits
    - pin to y-axis min/max (no padding for y-axis implemented yet)
    - scroll for Y axis zoom
    - hold shift when scrolling to make it x-axis zoom
    - have a default zoom mode (centered (i.e. mouse point) or pinned to axis)
    - switch zoom mode by holding CTRL when zooming
    - left / right axis mode

    It should be possible to remap keys. Need to connect options to completely
    customise every aspect of the plots.
*/
{

public:
    Camera(
        Configs& configs,
        LinkedSubplot& subplot,
        WindowViewportObject& windowViewport,
        JointPlotData& jointPlotData
    );
	~Camera();

    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
    Camera(Camera&&) = delete;
    Camera& operator=(Camera&&) = delete;

	glm::mat4 getNDCMatrix();

	double getLeft() const { return m_left; }
	double getRight() const { return m_right; }
	double getTop() const { return m_top; }
	double getBottom() const { return m_bottom; }

    double getAspectRatio() const;
    double getViewWidth() const;
    double getViewHeight() const;

	void resetYAxis();
	void resetXAxis();

    void processKeyboardPressY(const std::unordered_map<int, bool>& keyStates);
    void processKeyboardPressX(const std::unordered_map<int, bool>& keyStates);
    void leftMouseMove(double x, double y, bool changeY);
    void rightMouseMove(double x, double y, bool changeY);

    void wheelScrollZoom(double dy, std::optional<double> xMouseProportion, std::optional<double> yMouseProportion);
    void fixZoomToEdge(bool on, bool rightMouseButtonPressed);

    void setupView();
    void setYLimitsFromView();

    void storeViewClickPosition(double xClickPos, double clickPosPercent);
    void ensureZoomSwitchedModeOff();

    double getXValueUnderMouse(double xMouseProportion);
    int tickIndexUnderMouse(double xMouseProportion);
    double getYValueUnderMouse(double yMouseProportion);
    std::string getXLabelUnderMouse(double xMouseProportion);

    void zoomX(double zoomSpeed, double focusX, double xProportion);
    void zoomY(double zoomSpeed, double focusY);

    // Camera bounds
    double m_left;
    double m_right;
    double m_top;
    double m_bottom;

private:

    Configs& m_configs;
    LinkedSubplot& m_sp;
    WindowViewportObject& m_windowViewport;
    JointPlotData& m_jointPlotData;

    // TODO: holding the mouse-state here is vestigial
    // and at least some of this functionality would benefit
    // by being moved up to the CentralOpenGlWidget.
    MouseState m_mouseState;

    void initXView();

	void panX(double panValue);
    void panY(double panValue, bool applyAspectRatio);

    double getPadding();

	bool m_zoomModeSwitched = false;
	bool m_ctrlPressed = false;
};
