#include <QOpenGLVersionFunctionsFactory>

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLContext>
#include <QOpenGLWidget>
#include <QPainter>
#include <QTimer>
#include <qevent.h>
#include "CentralOpenGlWidget.h"
#include "../charts/plots/CandlestickPlot.h"
#include "../charts/plots/LinePlot.h"
#include <qlibrary.h>


CentralOpenGlWidget::CentralOpenGlWidget(QWidget *parent, Configs& configs)
    : QOpenGLWidget(parent),
    m_configs(configs),
    m_mainWindow(parent),
    m_crosshairSettings(configs.m_defaultCrosshairSettings),
    m_hoverValueSettings(configs.m_defaultHoverValueSettings),
    m_drawLineSettings(configs.m_defaultDrawLineSettings),
    m_timer(new QTimer(this))
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    connect(m_timer, &QTimer::timeout, this, [this]{ this->update(); });

}


void CentralOpenGlWidget::initializeGL()
{
    m_gl = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(QOpenGLContext::currentContext());

    m_gl->initializeOpenGLFunctions();

    m_rm = std::make_unique<RenderManager>(*this, m_configs, *m_gl);

    // If the mouse is not over the plot,  the gridlines don't update. This is a weird
    // experience for the user (the gridlines only appear when the mouse is moved over
    // the plot. So, set a time on the update to run just after initialisation to ensure
    // all gridlines are set.
    m_timer->start(16);
    QTimer::singleShot(500, this, [this]() {
        m_timer->stop();
    });
}


void CentralOpenGlWidget::paintGL()
{
    // Perform any previously qued actions that must be run in paintGL.
    while (!m_paintGlQueue.empty())
    {
        m_paintGlQueue.front()();
        m_paintGlQueue.pop();
    };

    // Handle zooming with left or right mousebuttons
    if (m_leftMouseButtonPressed || m_rightMouseButtonPressed)
    {
        QPoint pos = QCursor::pos();

        if (!m_lastPosForZoom.has_value())
        {
            m_lastPosForZoom = pos;
        }

        double dx = pos.x() - m_lastPosForZoom.value().x();
        double dy = pos.y() - m_lastPosForZoom.value().y();

        m_lastPosForZoom = pos;

        int plotIdx = 0;

        for (const std::unique_ptr<LinkedSubplot>& subplot : m_rm->m_linkedSubplots)
        {
            // Only zoom / pan Y on the hovered linked subplot, but we must zoom/pan X across the linked subplots
            bool changeY = m_configs.m_linkYZoomAndPan ? true : (plotIdx == m_mousePosInfo.lastClickedPlotIdx);

            if (m_leftMouseButtonPressed)
            {
                subplot->camera().leftMouseMove(dx, dy, changeY);
            }
            else if (m_rightMouseButtonPressed)
            {
                double magnitude = std::sqrt(dx * dx + dy * dy);
                double magnitudeCutoff = 145.0;

                if (magnitude > magnitudeCutoff)
                {
                    double scale = magnitudeCutoff / magnitude;
                    dx *=scale;
                    dy *= scale;
                }

                subplot->camera().rightMouseMove(dx, dy, changeY);
            }
            plotIdx += 1;
        }
    }

    if (m_keyStates[Qt::Key_W] || m_keyStates[Qt::Key_S] || m_keyStates[Qt::Key_Q] || m_keyStates[Qt::Key_E])
    {
        hoveredLinkedSubplot()->camera().processKeyboardPressY(m_keyStates);
    }

    if (m_keyStates[Qt::Key_A] || m_keyStates[Qt::Key_D] || m_keyStates[Qt::Key_Z] || m_keyStates[Qt::Key_C])
    {
        for (const std::unique_ptr<LinkedSubplot>& subplot: m_rm->m_linkedSubplots)
        {
            subplot->camera().processKeyboardPressX(m_keyStates);
        }
    }


    m_rm->paint();

    QPainter painter(this);

   if (m_showPopup && m_hoverValueSettings.displayMode != HoverValueDisplayMode::off)
    {
        showValuePopup(painter);
    }
  //  if (m_showCrosshair && m_crosshairSettings.on)
  //  {
  //      showCrosshairs(painter);
   // }
    painter.end();
}


void CentralOpenGlWidget::resizeGL(int width, int height)
{
    m_rm->updateWindowSize(width, height);
    m_gl->glViewport(0, 0, width, height);
    const qreal dpr = devicePixelRatio();

    // Convert logical size → physical size
    const GLsizei physicalWidth  = static_cast<GLsizei>(std::round(width  * dpr));
    const GLsizei physicalHeight = static_cast<GLsizei>(std::round(height * dpr));

    // Update any window-size dependent resources
    m_rm->updateWindowSize(physicalWidth, physicalHeight);

    // Set OpenGL viewport using physical pixels
    m_gl->glViewport(0, 0, physicalWidth, physicalHeight);

 //   m_rm->updateWindowSize(width, height);
 //   m_gl->glViewport(0, 0, width, height);
}

/* ------------------------------------------------------------------------------
    Mouse / Keyboard events
 * --------------------------------------------------------------------------- */


void CentralOpenGlWidget::keyPressEvent(QKeyEvent *event)
/*
    Handle key-press events, typically acting as modifier keys.
    Zoom / Pan keys trigger a timer that updates the GUI while key is pressed.
 */
{
    m_keyStates[event->key()] = true;

    if (!m_timer->isActive())
    {
        if (m_zoomPanKeys.contains(event->key()))
        {
            m_timer->start(8);
        }
    }

    // Now handle other non zoom / pan shortcuts
    std::tuple<int, double> result = getYMousePositionInfo();
    int plotIdx = std::get<0>(result);

    // Reset the view
    if (m_keyStates[Qt::Key_R])
    {
        for (const std::unique_ptr<LinkedSubplot>& subplot : m_rm->m_linkedSubplots)
        {
            subplot->camera().resetXAxis();
            subplot->camera().resetYAxis();
        }
    }

    // Toggle candlestick plot types
    if (m_keyStates[Qt::Key_Enter] || m_keyStates[Qt::Key_Return])
    {
       m_rm->m_linkedSubplots[plotIdx]->jointPlotData().cycleCandlestickPlotType();
    }

    // Show / hide gridlines
    if (m_keyStates[Qt::Key_G])
    {
        m_rm->m_linkedSubplots[plotIdx]->toggleXGridlines();
        m_rm->m_linkedSubplots[plotIdx]->toggleYGridlines();
    }

    // If CTRL is pressed, zoom is fixed to the edge (whatever the side of the axis)
    if (event->key() == Qt::Key_Control)
    {
        bool on = true;
        for (const std::unique_ptr<LinkedSubplot>& subplot : m_rm->m_linkedSubplots)
        {
            subplot->camera().fixZoomToEdge(on, m_rightMouseButtonPressed);
        }
    }

    // CTRL+S being pressed will
    if ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_S))
    {
        m_hoverValueStartPos += 1;
    }

   QOpenGLWidget::keyPressEvent(event);
   update();
}


void CentralOpenGlWidget::keyReleaseEvent(QKeyEvent *event)
{
    m_keyStates[event->key()] = false;

    if (!anyButtonPressed())
    {
        m_timer->stop();
    }

    // Turn off the edge zoom mode for all linked subplots
    if (event->key() == Qt::Key_Control)
    {
        bool on = false;
        for (const std::unique_ptr<LinkedSubplot>& subplot : m_rm->m_linkedSubplots)
        {
            subplot->camera().fixZoomToEdge(on, m_rightMouseButtonPressed);
        }
    }

    QOpenGLWidget::keyReleaseEvent(event);
}


void CentralOpenGlWidget::wheelEvent(QWheelEvent* event)
/*
    Zoom the plot that is currently under the mouse cursor.
 */
{
    std::tuple<int, double> result = getYMousePositionInfo();

    int plotIdx = std::get<0>(result);

    if (plotIdx == -1)
    {
        return;
    }

    double dy = event->angleDelta().y();

    // First zoom the Y on the hovered plot
    if (m_keyStates[Qt::Key_Shift])
    {
        m_rm->m_linkedSubplots[plotIdx]->camera().wheelScrollZoom(dy, std::nullopt, m_mousePosInfo.yMouseProportion);
    }

    if (!m_keyStates[Qt::Key_Shift])
    {
        // then update X on all linked subplots
        for (const std::unique_ptr<LinkedSubplot>& subplot : m_rm->m_linkedSubplots)
        {
            subplot->camera().wheelScrollZoom(dy, m_mousePosInfo.xMouseProportion, std::nullopt);
        }
    }

    update();
    QOpenGLWidget::wheelEvent(event);
}


void CentralOpenGlWidget::mousePressEvent(QMouseEvent *event)
/*
    Mouse click events are used to initialise panning
    (left) or zooming (right) behaviour or draw items (in draw mode).
 */
{

    m_mousePosInfo.lastClickPositionScreen = QCursor::pos();

    // Calc the percentage of the y-axis on which the click falls on
    // the clicked-on plot. When zooming is fixed across all subplots plots,
    // use this percentage to determine the click-position on the subplots
    // for accurate zooming. Note this is duplicated with the hover-info,
    // but we want to be 100% we have the right position and may not if the
    // events are not completely aligned.
    std::tuple<int, double> yClickInfo = getYMousePositionInfo(event->position().y());
    m_mousePosInfo.lastClickedPlotIdx = std::get<0>(yClickInfo);
    double clickPosPercent = std::get<1>(yClickInfo);

    if (m_mousePosInfo.lastClickedPlotIdx == -1)
    {
        return;
    }

    if (event->button() == Qt::LeftButton && m_keyStates.at(Qt::Key_M))
    {
        handleDrawModeClick();
        return;
    }

    // Zooming (zoom around mouse click point)
    if (event->button() == Qt::RightButton)
    {
        m_rightMouseButtonPressed = true;
        for (const std::unique_ptr<LinkedSubplot>& subplot : m_rm->m_linkedSubplots)
        {
            subplot->camera().storeViewClickPosition(
                getXMousePositionAsProportion(event->position().x() * devicePixelRatio()),
                clickPosPercent
            );
        }
    }

    // Panning (track graph displacement to correct mouse position)
    if (event->button() == Qt::LeftButton)
    {
        m_leftMouseButtonPressed = true;
    }

    m_showPopup = false;
    m_showCrosshair = false;

    QOpenGLWidget::mousePressEvent(event);
}


void CentralOpenGlWidget::mouseReleaseEvent(QMouseEvent *event)
{
    // End zoom
    if (m_rightMouseButtonPressed && event->button() == Qt::RightButton)
    {
        m_rightMouseButtonPressed = false;
        m_rm->m_linkedSubplots[m_mousePosInfo.lastClickedPlotIdx]->camera().ensureZoomSwitchedModeOff();
    }

    // End pan
    if (!m_keyStates.at(Qt::Key_M) && event->button() == Qt::LeftButton)
    {
        m_leftMouseButtonPressed = false;
    }

    m_showPopup = true;
    m_showCrosshair= true;

    // For axis tick updating after a big zoom with
    // some lag with mouse ending off the chart
   // if (m_mousePosInfo.m_hoverPlotIdx == -1)
   // {
    if (!m_timer->isActive())
    {
        m_timer->start(16);
        QTimer::singleShot(750, this, [this]() {
            m_timer->stop();
        });
    }
 //   }
    QOpenGLWidget::mouseReleaseEvent(event);

    m_lastPosForZoom = std::nullopt;
}


void CentralOpenGlWidget::enterEvent(QEnterEvent *event)
{
    m_showCrosshair= true;
    m_showPopup = true;
    QOpenGLWidget::enterEvent(event);
    update();
}


void CentralOpenGlWidget::leaveEvent(QEvent *event)
{
    m_mousePosInfo.m_hoverPlotIdx = -1;
    m_showCrosshair= false;
    m_showPopup = false;
    update();
    QOpenGLWidget::leaveEvent(event);
}


void CentralOpenGlWidget::mouseMoveEvent(QMouseEvent* event)
/*
    Store the current mouse position and plot data under the mouse, and trigger paintGL.
 */
{
    m_mousePosInfo.cursorPos = event->pos();
    auto yInfo = getYMousePositionInfo(m_mousePosInfo.cursorPos.y());

    m_mousePosInfo.m_hoverPlotIdx = std::get<0>(yInfo);
    double yMouseProportion = std::get<1>(yInfo);
    double xMouseProportion = getXMousePositionAsProportion(m_mousePosInfo.cursorPos.x() * devicePixelRatio());

    m_mousePosInfo.xMouseProportion = xMouseProportion;
    m_mousePosInfo.yMouseProportion = yMouseProportion;

    if (m_mousePosInfo.m_hoverPlotIdx != -1)
    {
        m_mousePosInfo.yData = hoveredLinkedSubplot()->camera().getYValueUnderMouse(yMouseProportion);
        m_mousePosInfo.xData = hoveredLinkedSubplot()->camera().getXValueUnderMouse(xMouseProportion);
        m_mousePosInfo.xTickLabel = hoveredLinkedSubplot()->camera().getXLabelUnderMouse(xMouseProportion);
        m_mousePosInfo.xIdx = hoveredLinkedSubplot()->camera().tickIndexUnderMouse(xMouseProportion);
    }

    if (m_keyStates.at(Qt::Key_M))
    {
        moveDrawLine();
    };

    update();

    // QOpenGLWidget::mouseMoveEvent(event);
}

/* ------------------------------------------------------------------------------
    Draw mode
 * --------------------------------------------------------------------------- */

void CentralOpenGlWidget::handleDrawModeClick()
{
    if (m_drawModeClicks == 0)
    {
        startDrawLine();
        m_drawModeClicks += 1;
    }
    else if (m_drawModeClicks == 1)
    {
        endDrawLine();
        m_drawModeClicks = 0;
    }
}


void CentralOpenGlWidget::startDrawLine()
{
    m_paintGlQueue.push(
        [this]() {
            this->m_rm->m_linkedSubplots[this->m_mousePosInfo.lastClickedPlotIdx]->startLineDraw(
                this->m_mousePosInfo.xData, this->m_mousePosInfo.yData, this->m_drawLineSettings
            );
        }
    );
}

void CentralOpenGlWidget::endDrawLine()
{
    m_paintGlQueue.push(
        [this]() {
            int plotIdx = this->m_mousePosInfo.lastClickedPlotIdx;
            this->m_rm->m_linkedSubplots[plotIdx]->endLineDraw();
        }
        );
}


void CentralOpenGlWidget::moveDrawLine()
{
    m_paintGlQueue.push(
        [this](){
            int plotIdx = this->m_mousePosInfo.m_hoverPlotIdx;
            m_rm->m_linkedSubplots[plotIdx]->mouseMoveLineDrawA(m_mousePosInfo.xData, m_mousePosInfo.yData);
        }
    );
}


/* ------------------------------------------------------------------------------
    Paint crosshairs and hover-value over plot (Qt side)
 * --------------------------------------------------------------------------- */

void CentralOpenGlWidget::showCrosshairs(QPainter& painter)
/*

 */
{
    // QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    // Draw crosshair lines
    glm::vec4 lcol = m_crosshairSettings.lineColor;
    QColor lineColor = QColor(lcol[0] * 255, lcol[1] * 255, lcol[2] * 255, lcol[3] * 255);  // TODO: GLM TO QT COLOR!
    QPen crosshairPen(lineColor, m_crosshairSettings.linewidth, Qt::DashLine);
    painter.setPen(crosshairPen);
    painter.drawLine(m_mousePosInfo.cursorPos.x(), 0, m_mousePosInfo.cursorPos.x(), height());
    painter.drawLine(0, m_mousePosInfo.cursorPos.y(), width(), m_mousePosInfo.cursorPos.y());

    QString xLabel = QString::fromStdString(m_mousePosInfo.xTickLabel);
    QString yLabel = QString("%1").arg(m_mousePosInfo.yData, 0, 'f', 3);

    // Set font
    QFont font = utils_getQtFont(m_crosshairSettings.font);
    font.setPointSize(m_crosshairSettings.fontSize);
    font.setStyleStrategy(QFont::PreferAntialias);
    painter.setFont(font);

    glm::vec4 pcol = m_crosshairSettings.fontColor;
    painter.setPen(QColor(pcol[0] * 255, pcol[1] * 255, pcol[2] * 255, pcol[3] * 255));

    glm::vec4 bcol = m_crosshairSettings.backgroundColor;
    QColor backgroundColor(bcol[0] * 255, bcol[1] * 255, bcol[2] * 255, bcol[3] * 255);
    painter.setBrush(backgroundColor);

    // X label (bottom)
    QRect xTextRect = painter.boundingRect(QRect(), Qt::AlignCenter, xLabel);

    xTextRect.moveCenter(QPoint(m_mousePosInfo.cursorPos.x(), height() - m_configs.m_plotOptions.heightMarginSize + xTextRect.height() / 2 - +1));
    xTextRect.setHeight(m_configs.m_plotOptions.heightMarginSize - 1);
    painter.drawRect(xTextRect);
    painter.drawText(xTextRect, Qt::AlignCenter, xLabel);

    // Y label (right)
    QRect yTextRect = painter.boundingRect(QRect(), Qt::AlignCenter, yLabel);
    yTextRect.setWidth(m_configs.m_plotOptions.widthMarginSize - 1);
    yTextRect.moveCenter(QPoint(width() -  m_configs.m_plotOptions.widthMarginSize + yTextRect.width() / 2 + 1, m_mousePosInfo.cursorPos.y()));
    painter.drawRect(yTextRect);
    painter.drawText(yTextRect, Qt::AlignCenter, yLabel);

    // painter.end();
}


void CentralOpenGlWidget::showValuePopup(QPainter& painter)
/*

 */
{
    {
        // Ignore out-of plot mouse positions
        if (m_mousePosInfo.m_hoverPlotIdx == -1 || m_mousePosInfo.xMouseProportion >= 1.0 || m_mousePosInfo.xMouseProportion < 0)
        {
            return;
        }

        // Fix a small padding around the plot within which the popup is still triggered by the mouse
        double viewHeight = hoveredLinkedSubplot()->camera().getViewHeight();
        double yPadding = viewHeight * 0.025;

        /*
        // First, cylce through all drawn lines (treat them as on top of plots) and get the plot data under the mouse
        const std::vector<std::unique_ptr<DrawLine>>& drawnVector = hoveredLinkedSubplot()->drawLines();

        bool alwaysShow = (m_hoverValueSettings.displayMode == HoverValueDisplayMode::alwaysShow);

        std::optional<UnderMouseData> info;

        bool useDrawn = false;
        for (int i = drawnVector.size() - 1; i >= 0; i--)
        {
            info = drawnVector[i]->getDataUnderMouse(m_mousePosInfo.xData, m_mousePosInfo.yData, yPadding, false);  // the lines are always shown on hover

            if (info.has_value())
            {
                useDrawn = true;
                break;
            }
        }
        */
        // Otherwise, cycle through all plots (on top plots first, so go backwards through the plots) and check for under-mouse data

        bool useDrawn = false;
        bool alwaysShow = true;
        std::optional<UnderMouseData> info;
        if (!useDrawn)
        {
            const std::vector<std::unique_ptr<BasePlot>>& plotVector = hoveredLinkedSubplot()->jointPlotData().plotVector();

            if (m_hoverValueStartPos > plotVector.size() || !alwaysShow)  // reset to first plot if we have gone over.
            {
                m_hoverValueStartPos = 1;
            }

            for (int i = plotVector.size() - m_hoverValueStartPos; i >= 0; i--)
            {
                if (const auto& plot = dynamic_cast<LinePlot*>(plotVector[i].get()))
                {
                    info = plot->getPlotData().getDataUnderMouse(m_mousePosInfo.xIdx, m_mousePosInfo.yData, yPadding, alwaysShow, m_mousePosInfo.xData);
                }
                else
                {
                    info = plotVector[i]->getPlotData().getDataUnderMouse(m_mousePosInfo.xIdx, m_mousePosInfo.yData, yPadding, alwaysShow);
                }
                if (info.has_value())
                {
                    break;
                }
            }
        }

        // If a plot was under the mouse, show the value in the pop-up
        if (info.has_value())
        {
            // 1. Build the text
            QString text;
            if (info.value().isCandle()) {
                const CandleInfo c = info.value().getCandleInfo();
                text = QString("Open: %1\nHigh: %2\nLow: %3\nClose: %4")
                           .arg(c.open,  0, 'f', 2)
                           .arg(c.high,   0, 'f', 2)
                           .arg(c.low,   0, 'f', 2)
                           .arg(c.close, 0, 'f', 2);
            } else {
                text = QString("%1").arg(info.value().getYData(), 0, 'f', 2);
            }

            // 2. Prepare painter
            // QPainter painter(this);
       //     painter.setRenderHint(QPainter::Antialiasing);
      //      painter.setRenderHint(QPainter::TextAntialiasing);

            const glm::vec4 bk = m_hoverValueSettings.backgroundColor;
       //     painter.setBrush(QColor(bk[0]*255, bk[1]*255, bk[2]*255, bk[3]*255));

            QFont font = utils_getQtFont(m_hoverValueSettings.font);
            font.setPointSize(m_hoverValueSettings.fontSize);
       //     painter.setFont(font);

            QFontMetrics fm(font);

            // 3. Measure each line (newline-safe)
            QStringList lines = text.split('\n');

            int maxWidth   = 0;
            int totalHeight = 0;
            for (const QString& line : lines) {
                QRect r = fm.boundingRect(line);         // one single line
                maxWidth    = std::max(maxWidth, r.width());
                totalHeight += fm.height();              // lineSpacing is safer than r.height()
            }

            // 4. Build the popup rectangle with padding
            const int pad = 5;

            double boxWidth = maxWidth + 2*pad;
        //    QSize popupSize(boxWidth, totalHeight + 2*pad);
       //     QPoint basePos = m_mousePosInfo.cursorPos + QPoint(-10 - boxWidth, 10);
      //      QRect  box(basePos, popupSize);

            // optional: clamp box so it never leaves the window
     //       if (box.left()  < 0.0)  box.moveRight(box.width() );
       //     if (box.bottom() > height())  box.moveTop (height() - box.height());

            // 5. Draw background & border
            const glm::vec4 bc = m_hoverValueSettings.borderColor;
        //    painter.setPen(QColor(bc[0]*255, bc[1]*255, bc[2]*255, bc[3]*255));
        //    painter.drawRect(box);

            // 6. Draw each line (guaranteed to fit)
            const glm::vec4 fc = m_hoverValueSettings.fontColor;
        //    painter.setPen(QColor(fc[0]*255, fc[1]*255, fc[2]*255, fc[3]*255));

            int y = box.top() + pad + fm.ascent();
            for (const QString& line : lines) {
       //         painter.drawText(box.left() + pad, y, line);
                y += fm.height();
            }

            // painter.end();
            m_showPopup = true;
        }
    }
}


/* ------------------------------------------------------------------------------
    Window position info
 * --------------------------------------------------------------------------- */

std::tuple<int, double> CentralOpenGlWidget::getYMousePositionInfo(std::optional<double> yPosition)
/*
    When the mouse is clicked, it will be on a particular subplot. Determine
    which subplot the click was on.

    Parameters
    ----------

    yPosition :
        y position in widgets coordinates of the mouse.

    Returns
    -------

    tuple<int, double>
        The first element is the index of the clicked on plot (0 is top)
        The second is the proportion along the plot y-axis that was clicked
        (1 the top, 0 the bottom)
*/
{
    double yPos;
    if (yPosition.has_value())
    {
        yPos = yPosition.value();
    }
    else
    {
        QPoint globalPos = QCursor::pos(); // Global screen coordinates
        QPoint localPos = mapFromGlobal(globalPos); // Relative to this widget
        yPos = (double)localPos.y();
    }

    double yPosProportion = 1.0 - (yPos / static_cast<double>(height()));

    double yMargin = m_rm->m_windowViewport.yMarginAsProportion();

    double yPosWithMargin = (yPosProportion - yMargin) / (1.0 - yMargin);

    int idx = 0;
    for (const std::unique_ptr<LinkedSubplot>& subplot : m_rm->m_linkedSubplots)
    {
        if (subplot->m_yStartProportion < yPosWithMargin && yPosWithMargin < subplot->m_yStartProportion + subplot->m_yHeightProportion)
        {
            double proportionOfPlot = (yPosWithMargin - subplot->m_yStartProportion) / subplot->m_yHeightProportion;

            return std::tuple<int, double>(idx, proportionOfPlot);
        }
        else
        {
            idx += 1;
        }
    }
    return std::tuple<int, double>(-1, 0.0);
}


double CentralOpenGlWidget::getXMousePositionAsProportion(double xPosition)
/*
    Get the x-position of the click as a proportion of the widget (from left axis).
    The proportion is useful because we zoom differently if the click is
    towards the very edge of the window.
*/
{
    int width = m_rm->m_windowViewport.getWindowWidth();

    double xMargin = m_configs.m_plotOptions.widthMarginSize * devicePixelRatio();

    double xMarginCompensation = (m_configs.m_plotOptions.axisRight) ? 0.0 : xMargin;

    double clickXPosAsProportion = (xPosition - xMarginCompensation) / ((double)width - xMargin);

    return clickXPosAsProportion;
}


void CentralOpenGlWidget::setLegend(
    std::variant<std::vector<std::string>, std::vector<LegendItem>> labels,
    int linkedSubplotIdx,
    LegendSettings legendSettings
)
/*
    Push to the queue to create a legend in the pinked subplots (required to be
    on the paint queue as this sets up OpenGL buffers when creating the legend).
 */
{
    BackendLegendSettings settings = m_configs.convertBackendLegendSettings(legendSettings);

    m_paintGlQueue.push(
        [this, labels, settings, linkedSubplotIdx]() {

            m_rm->m_linkedSubplots[linkedSubplotIdx]->setLegend(labels, settings);
        }
    );
}


std::unique_ptr<LinkedSubplot>& CentralOpenGlWidget::hoveredLinkedSubplot()
/*
    Get the subplot which is currently under the mouse.
 */
{
    return m_rm->m_linkedSubplots[m_mousePosInfo.m_hoverPlotIdx];
}


QSize CentralOpenGlWidget::getMainwindowSize()
/*
    Get the size of the entire window (in which this subplot resides)
 */
{
    return m_mainWindow->size();
}


/* ------------------------------------------------------------------------------
    Settings
 * --------------------------------------------------------------------------- */

void CentralOpenGlWidget::setCrosshairSettings(CrosshairSettings crosshairSettings)
{
    m_crosshairSettings = m_configs.convertToCrosshairSettings(crosshairSettings);
}


void CentralOpenGlWidget::setHoverValueSettings(HoverValueSettings hoverValueSettings)
{
    m_hoverValueSettings = m_configs.convertBackendHoverValueSettings(hoverValueSettings);
}


void CentralOpenGlWidget::setDrawLineSettings(DrawLineSettings drawLineSettings)
{
    m_drawLineSettings = m_configs.convertToBackendDrawLineSettings(drawLineSettings);
}


bool CentralOpenGlWidget::anyButtonPressed()
{
    for (const auto& it : m_keyStates)
    {
        if (it.second)
        {
            return true;
        }
    }
    return false;
}
