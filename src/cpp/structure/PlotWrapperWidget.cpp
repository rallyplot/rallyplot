#include "PlotWrapperWidget.h"
#include "../Utils.h"


void PlotWrapperWidget::setBackgroundColor(glm::vec4 color)
{
    m_backgroundColor = color;
    QString stylesheet = utils_glmBackgroundColorToQtStylesheet(color);
    this->setStyleSheet(stylesheet);
}

PlotWrapperWidget::PlotWrapperWidget(QWidget* parent, Configs configs) : QWidget(parent), m_configs(configs)
/*
    Set up all (initially empty) plot and axis labels, and instantiate the CentralOpenGlWidget.
 */
{
    setBackgroundColor(m_configs.m_backgroundColor);

    // Grid to hold the plot and  axis labels
    QGridLayout* grid = new QGridLayout(this);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(0);

    // Central OpenGL widget
    m_centralOpenGlWidget = new CentralOpenGlWidget(this, m_configs);
    m_centralOpenGlWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Plot title
    m_plotTitleLabel = new QLabel("", this);
    m_plotTitleLabel->setVisible(false);
    m_plotTitleLabel->setAlignment(Qt::AlignHCenter);
    m_plotTitleLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    // X Axis label
    m_xAxisLabel = new QLabel("", this);
    m_xAxisLabel->setVisible(false);
    m_xAxisLabel->setAlignment(Qt::AlignHCenter);
    m_xAxisLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    // Y Axis label
    bool clockwise = (m_configs.m_plotOptions.axisRight) ? true : false;
    m_yAxisLabel = new VerticalLabel("", this, clockwise);
    m_yAxisLabel->setVisible(false);
    m_yAxisLabel->setAlignment(Qt::AlignVCenter);
    m_yAxisLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    m_topPadWidget = new QWidget(this);
    m_bottomPadWidget = new QWidget(this);

    // Grid layout positions
    grid->addWidget(m_plotTitleLabel, 0, 1);
    grid->addWidget(m_centralOpenGlWidget, 1, 1);
    grid->addWidget(m_xAxisLabel, 2, 1);
    int yLabelSide = (m_configs.m_plotOptions.axisRight) ? 2 : 0;
    grid->addWidget(m_yAxisLabel, 1, yLabelSide);
    grid->addWidget(m_topPadWidget, 0, yLabelSide);
    grid->addWidget(m_bottomPadWidget, 2, yLabelSide);

    // Stretch from the center
    grid->setRowStretch(1, 1);
    grid->setColumnStretch(1, 1);

    this->show();

};


QString PlotWrapperWidget::glmTextColorToStylesheet(std::optional<std::vector<float>> color, std::optional<glm::vec4> defaultColor)
/*
    This converts a text color to stylesheet. By default, we reduce the alpha slightly to match that
    as rendered in OpenGL. ttf fonts are rendered in the shader based on their alpha (see font shader).
 */
{
    glm::vec4 glmColor;
    int alphaMultiple;
    if (!color.has_value())
    {
        glmColor = defaultColor.value();
        alphaMultiple = 175; // due to fft alpha issue (see above)
    }
    else
    {
        glmColor = utils_colorToGlmVec(color.value().data(), color.value().size());
        alphaMultiple = 255;
    }

    std::string sheet = (
        "color: rgba("
        + std::to_string(glmColor[0] * 255) + ", "
        + std::to_string(glmColor[1] * 255) + ", "
        + std::to_string(glmColor[2] * 255) + ", "
        + std::to_string(glmColor[3] * alphaMultiple) + ")"
        );

    return QString::fromStdString(sheet);
}


void PlotWrapperWidget::setXLabel(std::string text, AxisLabelSettings settings)
{
    setLabel(text, settings, m_xAxisLabel, m_configs.m_defaultXAxisSettings.fontColor);
};

void PlotWrapperWidget::setYLabel(std::string text, AxisLabelSettings settings)
{
    setLabel(text, settings, m_yAxisLabel, m_configs.m_defaultYAxisSettings.fontColor);
};

void PlotWrapperWidget::setTitle(std::string text, TitleLabelSettings settings)
{
    setLabel(text, settings, m_plotTitleLabel, m_configs.m_defaultXAxisSettings.fontColor);  // these defaults are just for light / dark mode
};

void PlotWrapperWidget::setLabel(std::string text, LabelSettingsBase settings, QLabel* axisLabelPtr, std::optional<glm::vec4> defaultColor)
/*
    Core function to set the text and font on a label.
 */
{
    QFont font = utils_getQtFont(settings.font);

    font.setWeight(utils_stringToQtFontWeight(QString::fromStdString(settings.weight)));
    font.setPointSize(settings.fontSize);

    axisLabelPtr->setFont(font);
    axisLabelPtr->setVisible(true);
    axisLabelPtr->setText(QString::fromStdString(text));
    axisLabelPtr->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    axisLabelPtr->setStyleSheet(glmTextColorToStylesheet(settings.color, defaultColor));
    setBackgroundColor(m_backgroundColor);
}


int PlotWrapperWidget::accountForNegativeIdx(int index) const
{
    int numSubplots = m_centralOpenGlWidget->m_rm->m_linkedSubplots.size();

    if (index < 0)
    {
        if (numSubplots + index < 0)
        {
            if (index < 0)
            {
                throw std::runtime_error("The negative index " + std::to_string(index) + "is bigger than the number of subplots: " + std::to_string(numSubplots));
            }
        }
        int processedIndex = numSubplots + index;
        return processedIndex;
    }
    else
    {
        return index;
    }
}

const std::unique_ptr<LinkedSubplot>& PlotWrapperWidget::linkedSubplot(int index) const
{

    index = accountForNegativeIdx(index);

    int numSubplots = m_centralOpenGlWidget->m_rm->m_linkedSubplots.size();

    if (index >= numSubplots)
    {
        throw std::runtime_error("The `linkedSubplotIdx`: " + std::to_string(index) + " is larger than the number of plots: " + std::to_string(numSubplots));
    }

    return m_centralOpenGlWidget->m_rm->m_linkedSubplots[index];
};
