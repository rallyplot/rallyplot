#include "Legend.h"
#include "../../structure/LinkedSubplot.h"
#include <qopenglfunctions_3_3_core.h>
#include <glm.hpp>


Legend::Legend(
        std::vector<LegendItemGlm> legendItems,
        LinkedSubplot& linkedSubplot,
        Configs& configs,
        QOpenGLFunctions_3_3_Core& glFunctions,
        BackendLegendSettings legendSettings
    )
    : m_legendItems(legendItems),
    m_linkedSubplot(linkedSubplot),
    m_gl(glFunctions),
    m_BoxVAO(glFunctions),
    m_OutlineVAO(glFunctions),
    m_textVAO(glFunctions),
    m_boxProgram("legend_vertex.shader", "legend_fragment.shader", glFunctions),
    m_textProgram("legend_text_vertex.shader", "font_fragment.shader", glFunctions),
    m_charTextureAtlas(glFunctions, GL_TEXTURE2, legendSettings.font, legendSettings.fontSize * linkedSubplot.pixelRatio()),
    m_cfg(legendSettings)
{
    setupBoxBuffer();
    setupTextBuffer();

    m_boxProgram.setupAndBindProgram();
    m_textProgram.setupAndBindProgram();

    setLabelsAndBoxSize();
}


void Legend::setLabelsAndBoxSize()
/*
    Set all labels into an array to write to buffer,
    using the final text sizes to configure the legend box.
 */
{
    std::vector<std::string> labels;
    m_numChars = 0;
    for (int i = 0; i < m_legendItems.size(); i++)
    {
        labels.push_back(m_legendItems[i].name);
        m_numChars += m_legendItems[i].name.size();
    }
    auto [labelPixelsWidth, labelPixelsHeight] = writeLabelsToBuffer(labels, m_numChars);

    m_labelPixelsWidth = labelPixelsWidth;
    m_labelPixelsHeight = labelPixelsHeight;

    m_boxWidth =  m_cfg.xLinePadLeft + labelPixelsWidth + m_cfg.lineWidth + m_cfg.xTextPadRight;
    m_boxHeight = m_cfg.yItemPad * 2 + labelPixelsHeight * m_legendItems.size() + m_cfg.yInterItemPad * (m_legendItems.size() - 1);
}

void Legend::setupBoxBuffer()
/*
    A simple box formed of two triangles, with the buffer
    including a flag indicating if the vertex is on the
    left or right edge of the box (used for color gradient).
 */
{
    m_BoxVAO.setup();

    std::array<float, 18> boxLayout = {
        // First triangle  (position (x, y), left or right (for color)
        0, 0,     0,
        1, 0,     1,
        0, 1,     0,

        // Second Triangle
        1, 0,     1,
        0, 1,     0,
        1, 1,     1
    };

    m_gl.glGenBuffers(1, &m_BoxVBO);
    m_gl.glBindBuffer(GL_ARRAY_BUFFER, m_BoxVBO);

    m_gl.glBufferData(GL_ARRAY_BUFFER, sizeof(boxLayout), boxLayout.data(), GL_STATIC_DRAW);

    m_gl.glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    m_gl.glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)2);

    m_gl.glEnableVertexAttribArray(0);
    m_gl.glEnableVertexAttribArray(1);
}


void Legend::draw(Camera& camera)
/*
    Draw the legend in three parts, the out box, the colored line (indicating
    the plot) as a box, and the text. Note that the line and text must be shifted
    along the y-axis based on the current label. For the line this is handled in a loop,
    but the text is stored in a single buffer avoid writing to the buffer every draw
    call. Therefore, the index is stored in the buffer and handled within the shader.
    This is not very nice at all as duplicates logic here and in the shader, but it is most efficient.
 */
{
    // Scale to pixels by the current window size (this results in the legend size being fixed across all window resizing)
    float xInitWindowScaler = 2.0f / (float)m_linkedSubplot.windowViewport().getWidthNoMargin() * m_cfg.legendSizeScaler;
    float yInitWindowScaler = 2.0f / (float)m_linkedSubplot.windowViewport().getHeightNoMargin() / m_linkedSubplot.m_yHeightProportion * m_cfg.legendSizeScaler;

    m_gl.glEnable(GL_BLEND);
    m_gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /*
        First, draw the outer box. This encodes the box width / height
        and the padding between the end of the plot and the box.
    */

    m_BoxVAO.bind();
    m_boxProgram.bind();

    m_boxProgram.setUniform1f("boxWidth", m_boxWidth * xInitWindowScaler);
    m_boxProgram.setUniform1f("boxHeight", m_boxHeight * yInitWindowScaler);

    m_boxProgram.setUniform4f("leftColor", m_cfg.boxColor);  // gradient unused for box
    m_boxProgram.setUniform4f("rightColor", m_cfg.boxColor);

    m_boxProgram.setUniform1f("widthPositionOffset", (m_boxWidth + m_cfg.xBoxPad) * xInitWindowScaler);
    m_boxProgram.setUniform1f("heightPositionOffset", (m_boxHeight + m_cfg.yBoxPad) * yInitWindowScaler);

    m_gl.glDrawArrays(GL_TRIANGLES, 0, 6);

    /*
        Next, draw all the text. The index of the text item is stored on the
        buffer (we do not loop, to avoid writing to buffer during draw calls).
        All positioning offsets are passed to the shader where they are scaled
        and combined with the label index. This is only relevant for the y dimension.
    */

    m_textProgram.bind();
    m_textVAO.bind();

    m_charTextureAtlas.activateAndBind();

    m_textProgram.setUniform1i("text", 2); // GL_TEXTURE2
    m_textProgram.setUniform4f("fontColor", m_cfg.fontColor);

    m_textProgram.setUniform1f("boxWidth",  m_boxWidth * xInitWindowScaler);
    m_textProgram.setUniform1f("boxHeight", m_boxHeight * yInitWindowScaler);

    m_textProgram.setUniform1f("yItemPad", m_cfg.yItemPad);
    m_textProgram.setUniform1f("yBoxPad", m_cfg.yBoxPad);
    m_textProgram.setUniform1f("labelPixelsHeight", m_labelPixelsHeight);
    m_textProgram.setUniform1f("yInterLinePad", m_cfg.yInterItemPad);

    m_textProgram.setUniform1f("xInitWindowScaler", xInitWindowScaler);
    m_textProgram.setUniform1f("yInitWindowScaler", yInitWindowScaler);

    m_textProgram.setUniform1f("widthPositionOffset", (
    //  pad into the box from the left | padding and the length of the line
        m_boxWidth + m_cfg.xBoxPad - m_cfg.xLinePadLeft - m_cfg.lineWidth - m_cfg.xLinePadRight + m_cfg.xTextPadRight) * xInitWindowScaler
    );

    m_gl.glDrawArrays(GL_TRIANGLES, 0, m_numChars * 6);

    /*
        Finally, draw the colored lines (to the left of the text).
        Iterate over each label and draw it.

        TODO: This is insanely waseful! It would be better to write
        this to a buffer just like text.
    */

    m_boxProgram.bind();
    m_BoxVAO.bind();

    m_boxProgram.setUniform1f("boxWidth", m_cfg.lineWidth * xInitWindowScaler);
    m_boxProgram.setUniform1f("boxHeight", m_cfg.lineHeight *  yInitWindowScaler);

    m_boxProgram.setUniform1f("widthPositionOffset", (m_boxWidth + m_cfg.xBoxPad - m_cfg.xLinePadLeft + m_cfg.xTextPadRight) * xInitWindowScaler);

    for (int i = 0; i < m_legendItems.size(); i++)
    {
        m_boxProgram.setUniform4f("leftColor", m_legendItems[i].leftColor);
        m_boxProgram.setUniform4f("rightColor", m_legendItems[i].rightColor);

        m_boxProgram.setUniform1f("heightPositionOffset", (
        //  top padding into the box | existing labels     |  space between labels | shift back half the text height | and half the height of the line
            m_cfg.yBoxPad + m_cfg.yItemPad + (m_labelPixelsHeight * i) + m_cfg.lineHeight + m_cfg.yInterItemPad * i + m_labelPixelsHeight / 2.0f  - m_cfg.lineHeight / 2.0f
        ) * yInitWindowScaler);

        m_gl.glDrawArrays(GL_TRIANGLES, 0, 6);
   }
    m_gl.glDisable(GL_BLEND);
}


std::pair<float, float> Legend::writeLabelsToBuffer(const std::vector<std::string> allLabelText, int numDigits)
/*
    Write all text labels to buffer to be written as textures in the text draw calls.
    Include the 'world' position (that determines how it is represented on screen),
    the index of the label and the characters position in the texture map.

    This is very similar to the AxisTickLabels writeTextToBuffer.
 */
{
    std::vector<float> vertices;  // use array!
    vertices.reserve(numDigits);

    int numLabels = allLabelText.size();
    std::cout << "NUM LABELS: " << numLabels << std::endl;

    std::vector<float> textWidths;
    float maxTextHeight = 0;

    for (int labelIndex = 0; labelIndex < numLabels; labelIndex++)
    {
        const std::string& label = allLabelText[labelIndex];

        int charXOffset = 0;  // position of the character in the labelIndex label

        // For each character, compute the offsets and store in the buffer
        for (const char& c : label)
        {
            Character ch =  m_charTextureAtlas.getCharacter(c);

            float xpos, ypos;

            xpos = charXOffset + ch.bearing.x;
            ypos = 0.0f - (ch.size.y - ch.bearing.y) - (float)m_charTextureAtlas.textYSize();

            if (maxTextHeight < std::abs(ch.size.y))
            {
                maxTextHeight = std::abs(ch.size.y);
            }

            // Compute the position of this characters texture in the texture atlas.
            float texelWidth = 1.0f / (float)m_charTextureAtlas.atlasWidth();
            float texLeft = (float)ch.xTexturePos / (float)m_charTextureAtlas.atlasWidth();;
            float texRight = ((float)ch.xTexturePos + (float)ch.size.x) / (float)m_charTextureAtlas.atlasWidth();;

            texLeft += 0.5 * texelWidth;
            texRight -= 0.5 * texelWidth;

            // Store everything in the buffer to
            // be passed to font_vertex_shader
            //                                x pos world              y pos world             labelIndex             texture x pos (NDC)     texture y pos (NDC)
            vertices.insert(vertices.end(), { xpos,                    ypos,                 (float)labelIndex,       texLeft,                (float)((double)ch.size.y / (double)m_charTextureAtlas.atlasHeight()) });
            vertices.insert(vertices.end(), { xpos,                    ypos + ch.size.y,     (float)labelIndex,       texLeft,                0.0f });
            vertices.insert(vertices.end(), { xpos + ch.size.x,        ypos + ch.size.y,     (float)labelIndex,       texRight,               0.0f });

            vertices.insert(vertices.end(), { xpos,                    ypos,                 (float)labelIndex,       texLeft,                (float)((double)ch.size.y / (double)m_charTextureAtlas.atlasHeight()) });
            vertices.insert(vertices.end(), { xpos + ch.size.x,        ypos + ch.size.y,     (float)labelIndex,       texRight,               0.0f });
            vertices.insert(vertices.end(), { xpos + ch.size.x,        ypos,                 (float)labelIndex,       texRight,               (float)((double)ch.size.y / (double)m_charTextureAtlas.atlasHeight()) });

            charXOffset += (ch.advance >> 6);
        }

        textWidths.push_back(charXOffset);
    }

    // Store the array in the buffer.
    m_textVAO.bind();

    m_gl.glBindBuffer(GL_ARRAY_BUFFER, m_textVBO);

    m_gl.glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        vertices.size() * sizeof(float),
        vertices.data()
    );

    return std::pair<float, float>(*std::max_element(textWidths.begin(), textWidths.end()), maxTextHeight);
}


void Legend::setupTextBuffer()
/*
    Setup the buffer for the tick labels, which contains information on
    their view position offsets to adjust around axes, and texture position.
*/
{
    m_textVAO.setup();

    std::array<float, 1000> initVector;
    initVector.fill(0.0f);

    m_gl.glGenBuffers(1, &m_textVBO);

    m_gl.glBindBuffer(GL_ARRAY_BUFFER, m_textVBO);

    m_gl.glBufferData(GL_ARRAY_BUFFER, sizeof(initVector), initVector.data(), GL_STATIC_DRAW);

    // Layout 0 is (x offset, y offset, tick index)
    m_gl.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // Layout 1 is (texture x position, texture y position)
    m_gl.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    m_gl.glEnableVertexAttribArray(0);
    m_gl.glEnableVertexAttribArray(1);

}




