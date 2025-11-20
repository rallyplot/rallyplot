// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <qlabel.h>
#include <qmainwindow.h>
#include <QObject>
#include <QMetaObject>
#include <queue>

#include "RenderManager.h"


class CentralOpenGlWidget : public QOpenGLWidget
{
    Q_OBJECT


public:
    CentralOpenGlWidget(QWidget *parent, Configs& configs);

    CentralOpenGlWidget(const CentralOpenGlWidget&) = delete;
    CentralOpenGlWidget& operator=(const CentralOpenGlWidget&) = delete;
    CentralOpenGlWidget(CentralOpenGlWidget&&) = delete;
    CentralOpenGlWidget& operator=(CentralOpenGlWidget&&) = delete;

    void setLegend(
        std::variant<std::vector<std::string>, std::vector<LegendItem>> labels,
        int subplotIndex,
        LegendSettings legendSettings
    );
    void setCrosshairSettings(CrosshairSettings crosshairSettings);
    void setHoverValueSettings(HoverValueSettings hoverValueSettings);
    void setDrawLineSettings(DrawLineSettings drawLineSettings);

    QSize getMainwindowSize();

    std::unique_ptr<RenderManager> m_rm;  // we manage this
    Configs& m_configs;

private:

    QWidget* m_mainWindow = nullptr;
    BackendCrosshairSettings m_crosshairSettings;
    BackendHoverValueSettings m_hoverValueSettings;
    BackendDrawLineSettings m_drawLineSettings;

    QOpenGLFunctions_3_3_Core* m_gl = nullptr;  // Qt manages this

    std::optional<QPoint> m_lastPosForZoom = std::nullopt;

    bool m_mouseOutOfBounds = true;  // TODO: this definately duplicates some stuff

    QTimer* m_timer;

    struct MousePosInfo
    {
        QPoint cursorPos;
        double yData;
        double xData;
        int xIdx;
        int m_hoverPlotIdx = -1;
        std::string xTickLabel;
        double xMouseProportion;
        double yMouseProportion;
        QPoint lastClickPositionScreen;
        int lastClickedPlotIdx;
    };
    MousePosInfo m_mousePosInfo;

    bool m_drawMode = false;
    int m_drawModeClicks = 0;
    bool m_showPopup = true;
    bool m_showCrosshair= false;
    int m_hoverValueStartPos = 1;

    std::queue<std::function<void()>> m_paintGlQueue;    

    void onFrameSwapped();

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void enterEvent(QEnterEvent  *event) override;

    void showValuePopup();
    void showCrosshairs();

    void startDrawLine();
    void endDrawLine();
    void moveDrawLine();
    void handleDrawModeClick();

    std::tuple<int, double> getYMousePositionInfo(std::optional<double> yClickPosition = std::nullopt);
    double getXMousePositionAsProportion(double xPosition);

    std::unique_ptr<LinkedSubplot>& hoveredLinkedSubplot();

    bool m_leftMouseButtonPressed = false;
    bool m_rightMouseButtonPressed = false;

    std::unordered_map<int, bool> m_keyStates = {
        {Qt::Key_W, false},
        {Qt::Key_A, false},
        {Qt::Key_S, false},
        {Qt::Key_D, false},
        {Qt::Key_Q, false},
        {Qt::Key_E, false},
        {Qt::Key_Z, false},
        {Qt::Key_C, false},
        {Qt::Key_M, false},
        {Qt::Key_Shift, false}
    };
    QList<int> m_zoomPanKeys = {  // TODO: .h
                              Qt::Key_Q, Qt::Key_W, Qt::Key_E,
                              Qt::Key_A, Qt::Key_S, Qt::Key_D,
                              Qt::Key_Z, Qt::Key_X, Qt::Key_C,
                              };
    bool anyButtonPressed();
};


#endif
