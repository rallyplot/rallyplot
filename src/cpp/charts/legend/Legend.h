#ifndef LEGEND_H
#define LEGEND_H

#include "../Camera.h"
#include "../CharTextureAtlas.h"
#include "../../opengl/VertexArrayObject.h"
#include "../plots/BasePlot.h"
#include "../shaders/Program.h"
#include <qopenglfunctions_3_3_core.h>

class LinkedSubplot;  // forward declaration


struct LegendItemGlm
/*
    Item containg the name and line color of the legend item. The line color
    may have a gradient (left, right colors). These can be passed directly,
    constructed from a LegendItem (user facing, using std::vector<float> rather
    than glm::vec4) or determined from the plot. For candlestick, the gradient is
    across the up and down colors, othereline they are the same (the color of the plot).
 */
{
    LegendItemGlm(const std::string& name_, const glm::vec4& left, const glm::vec4& right)
        : name(name_), leftColor(left), rightColor(right) {}

    LegendItemGlm(const LegendItem& legendItem)
    {
        name = legendItem.name;
        leftColor = utils_colorToGlmVec(legendItem.leftColor.data(), legendItem.leftColor.size());
        rightColor = utils_colorToGlmVec(legendItem.rightColor.data(), legendItem.rightColor.size());
    }

    LegendItemGlm(const std::string& name_, const std::unique_ptr<BasePlot>& plotOfLegend)
    {
        name = name_;

        if (const auto* plot = dynamic_cast<const OneValuePlot*>(plotOfLegend.get()))
        {
            PlotColor plotColor = plot->getPlotColor();
            leftColor = plotColor.color;
            rightColor = plotColor.color;
        }
        else if (const auto* plot = dynamic_cast<const FourValuePlot*>(plotOfLegend.get()))
        {
            CandlestickColor plotColor = plot->getPlotColor();
            leftColor = plotColor.upColor;
            rightColor = plotColor.downColor;
        }
        else
        {
            throw std::runtime_error("CRITICAL ERROR: Plot type not recognised.");
        }
    }

    std::string name;
    glm::vec4 leftColor;
    glm::vec4 rightColor;
};


class Legend
{
public:

    Legend(
        std::vector<LegendItemGlm> legendItem,
        LinkedSubplot& linkedSubplot,
        Configs& configs,
        QOpenGLFunctions_3_3_Core& glFunctions,
        BackendLegendSettings legendSettings
    );

    void draw(Camera& m_camera);

private:

    std::pair<float, float> writeLabelsToBuffer(const std::vector<std::string> allLabelText, int numDigits);
    void setupTextBuffer();
    void setupBoxBuffer();
    void setLabelsAndBoxSize();

    std::vector<LegendItemGlm> m_legendItems;

    int m_numChars;
    float m_labelPixelsWidth;
    float m_labelPixelsHeight;
    float m_boxWidth;
    float m_boxHeight;

    LinkedSubplot& m_linkedSubplot;
    QOpenGLFunctions_3_3_Core& m_gl;
    VertexArrayObject m_BoxVAO;
    VertexArrayObject m_OutlineVAO;
    VertexArrayObject m_textVAO;
    Program m_boxProgram;
    Program m_textProgram;
    CharTextureAtlas m_charTextureAtlas;
    BackendLegendSettings m_cfg;

    unsigned int m_BoxVBO;
    unsigned int m_textVBO;

};

#endif // LEGEND_H
