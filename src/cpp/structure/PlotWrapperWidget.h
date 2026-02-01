#ifndef PLOTWRAPPERWIDGET_H
#define PLOTWRAPPERWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QFont>
#include <glm.hpp>
#include "CentralOpenGlWidget.h"
#include "VerticalLabel.h"
#include "../include/Plotter.h"


class PlotWrapperWidget : public QWidget
/*
    This wrapper holds the plot and draws axes labels on the Qt side.
    The plot is in the center of a grid widget, with axis and plot
    labels drawn around the centeral plot using the grid layout.

    Note: see the note below on CentralOpenGlWidget's parent.
 */
{
    Q_OBJECT

public:

    explicit PlotWrapperWidget(QWidget* parent, Configs configs);

    void setXLabel(std::string text, AxisLabelSettings settings);
    void setYLabel(std::string text, AxisLabelSettings settings);
    void setTitle(std::string text, TitleLabelSettings settings);
    void setBackgroundColor(glm::vec4 color);

    const std::vector<std::unique_ptr<LinkedSubplot>>& allLinkedSubplots() const { return m_centralOpenGlWidget->m_rm->m_linkedSubplots; };

    int accountForNegativeIdx(int index) const;
    const std::unique_ptr<LinkedSubplot>& linkedSubplot(int index) const;

    Configs& configs() { return m_configs; };
    const SharedXData& sharedXData() const {return m_centralOpenGlWidget->m_rm->m_sharedXData; };
    CentralOpenGlWidget* openGlWidget() const { return m_centralOpenGlWidget; };
    const std::unique_ptr<RenderManager>& renderManager() const { return m_centralOpenGlWidget->m_rm; };

    CentralOpenGlWidget* m_centralOpenGlWidget;


private:

    // These pointers are to widgets which are children of
    // this widget, so clean up is left to Qt.
    QLabel* m_plotTitleLabel;
    QLabel* m_xAxisLabel;
    VerticalLabel* m_yAxisLabel;
    QWidget* m_topPadWidget;
    QWidget* m_bottomPadWidget;


    Configs m_configs;
    glm::vec4 m_backgroundColor;

    void setLabel(std::string text, LabelSettingsBase settings, QLabel* axisLabelPtr, std::optional<glm::vec4> defaultColor);
    QString glmTextColorToStylesheet(std::optional<std::vector<float>> color, std::optional<glm::vec4> defaultColor);

signals:
};

#endif // PLOTWRAPPERWIDGET_H

