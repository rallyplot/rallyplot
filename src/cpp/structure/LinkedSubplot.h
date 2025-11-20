#ifndef LINKEDSUBPLOT_H
#define LINKEDSUBPLOT_H

#include "SharedXData.h"
#include "WindowViewportObject.h"
#include "../charts/AxesObject.h"
#include "../charts/AxisTickLabels.h"
#include "../charts/Camera.h"
#include "../Configs.h"
#include "JointPlotData.h"
#include "../include/Plotter.h"
#include "../charts/drawing/DrawLine.h"
#include "../charts/legend/Legend.h"

class RenderManager;

class LinkedSubplot
{
public:
    LinkedSubplot(
        RenderManager& renderManager,
        Configs& configs,
        BackendCameraSettings subplotCameraSettings,
        BackendAxisSettings subplotXAxisSettings,
        BackendAxisSettings subplotYAxisSettings,
        SharedXData& sharedXData,
        QOpenGLFunctions_3_3_Core& glFunctions,
        WindowViewportObject& windowViewport,
        double yStartProportion,
        double yHeightProportion
    );
    ~LinkedSubplot() {};

    LinkedSubplot(const LinkedSubplot&) = delete;
    LinkedSubplot& operator=(const LinkedSubplot&) = delete;
    LinkedSubplot(LinkedSubplot&&) = delete;
    LinkedSubplot& operator=(LinkedSubplot&&) = delete;

    std::vector<std::unique_ptr<DrawLine>> m_drawLines;
    bool m_lineDrawing = false;
    void mouseMoveLineDrawA(double x, double y);
    void startLineDraw(double x, double y, BackendDrawLineSettings drawLineSettings);
    void endLineDraw();

    void candlestick(
        const float* openPtr, std::size_t openSize,
        const float* highPtr, std::size_t highSize,
        const float* lowPtr, std::size_t lowSize,
        const float* closePtr, std::size_t closeSize,
        OptionalDateVector date,
        BackendCandlestickSettings backendSettings
    );

    void line(
        const float* yPtr, std::size_t ySize,
        OptionalDateVector date,
        BackendLineSettings backendSettings
    );

    void bar(
        const float* yPtr, std::size_t ySize,
        OptionalDateVector date,
        BackendBarSettings backendSettings
    );

    void scatter(
        ScatterDateVector xData,
        const float* yPtr,
        std::size_t ySize,
        BackendScatterSettings backendSettings
    );

    void draw();

    Configs& configs() { return m_configs; };

    const BackendCameraSettings& cameraSettings() const {return m_linkedSubplotCameraSettings; };
    const BackendAxisSettings& xAxisSettings() const {return m_linkedSubplotXAxisSettings; };
    const BackendAxisSettings& yAxisSettings() const {return m_linkedSubplotYAxisSettings; };
    void toggleXGridlines() { m_linkedSubplotXAxisSettings.showGridline = ! m_linkedSubplotXAxisSettings.showGridline; };
    void toggleYGridlines() { m_linkedSubplotYAxisSettings.showGridline = ! m_linkedSubplotYAxisSettings.showGridline; };

    void setCameraSettings(CameraSettings cameraSettings )
    {
        m_linkedSubplotCameraSettings.core = cameraSettings;
    }
    void setXAxisSettings(BackendAxisSettings axisSettings)
    {
        m_linkedSubplotXAxisSettings = axisSettings;
    }
    void setYAxisSettings(BackendAxisSettings axisSettings)
    {
        m_linkedSubplotYAxisSettings = axisSettings;
    }

    void pinYAxis(bool on);
    void setYLimits(std::optional<float> min, std::optional<float> max);

    JointPlotData& jointPlotData() { return m_JointPlotData; };
    WindowViewportObject& windowViewport() { return m_windowViewport; };
    Camera& camera() { return m_camera; };
    AxesObject& axesObject() { return m_axesObject; };
    AxisTickLabels& axisTickLabels() { return m_axisTickLabels; };
    SharedXData& sharedXData() { return m_sharedXData; };
    const std::vector<std::unique_ptr<DrawLine>>& drawLines() { return m_drawLines; };

    // hold the position of the subplot on the plot
    // as a proportion of  plot height
    double m_yStartProportion;
    double m_yHeightProportion;

    void setLegend(
        std::variant<std::vector<std::string>, std::vector<LegendItem>> labels,
        BackendLegendSettings legendSettings
    );

    double pixelRatio() { return m_windowViewport.pixelRatio(); };

private:

    RenderManager& m_rm;
    Configs& m_configs;
    BackendCameraSettings m_linkedSubplotCameraSettings;
    BackendAxisSettings m_linkedSubplotXAxisSettings;
    BackendAxisSettings m_linkedSubplotYAxisSettings;
    SharedXData& m_sharedXData;
    QOpenGLFunctions_3_3_Core& m_gl;
    WindowViewportObject& m_windowViewport;
    JointPlotData m_JointPlotData;
    Camera m_camera;
    AxisTickLabels m_axisTickLabels;
    AxesObject m_axesObject;
    std::unique_ptr<Legend> m_legend = nullptr;

    void setupFirstPlot(int numElements);
    void updateYAxisLimits();
};

#endif // LINKEDSUBPLOT_H
