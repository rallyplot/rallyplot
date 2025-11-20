#include <qfileinfo.h>

#include "AxisTickLabels.h"
#include "../structure/LinkedSubplot.h"

#include <vector>
#include <QOpenGLFunctions_3_3_Core>

#include "shaders/Program.h"
#include "../opengl/VertexArrayObject.h"
#include "shaders/Program.h"
#include "../Configs.h"


AxisTickLabels::AxisTickLabels(Configs& configs, LinkedSubplot& subplot, QOpenGLFunctions_3_3_Core& glFunctions)
    : m_configs(configs),
    m_linkedSubplot(subplot),
    m_gl(glFunctions),
    m_fontProgram(
        "font_vertex.shader",
        "font_fragment.shader",
        m_gl
    ),
    m_xTickLabelVAO(m_gl),
    m_yTickLabelVAO(m_gl),
    m_charTextureAtlas(glFunctions, GL_TEXTURE0, configs.m_plotOptions.axisTickLabelFont, configs.m_plotOptions.axisTickLabelFontSize * subplot.pixelRatio())
{
    m_fontProgram.setupAndBindProgram();

    ySetupVAO();
    xSetupVAO();
}


AxisTickLabels::~AxisTickLabels() 
/*
	Buffer and texture clean up.
*/
{
    m_gl.glDeleteBuffers(1, &m_yLabelVBO);
    m_yLabelVBO = 0;

    m_gl.glDeleteBuffers(1, &m_xLabelVBO);
    m_xLabelVBO = 0;

    m_gl.glDisableVertexAttribArray(0);
    m_gl.glDisableVertexAttribArray(1);
}


/* --------------------------------------------------------------------------------------------------------
	Setup Buffers
----------------------------------------------------------------------------------------------------------*/


void AxisTickLabels::ySetupVAO()
{
	m_yTickLabelVAO.setup();
	setupVAO(m_yLabelVBO, m_yLabelBuffer);
	m_yTickLabelVAO.unBind();
}


void AxisTickLabels::xSetupVAO()
{
	m_xTickLabelVAO.setup();
	setupVAO(m_xLabelVBO, m_xLabelBuffer);
	m_xTickLabelVAO.unBind();
}


void AxisTickLabels::setupVAO(unsigned int& VBO, std::vector<float>& fontBuffer)
/*
	Setup the buffer for the tick labels, which contains information on
	their view position offsets to adjust around axes, and texture position.
*/
{
    m_gl.glGenBuffers(1, &VBO);

    m_gl.glBindBuffer(GL_ARRAY_BUFFER, VBO);

    m_gl.glBufferData(GL_ARRAY_BUFFER, m_maxEverBufferLength * sizeof(float), fontBuffer.data(), GL_DYNAMIC_DRAW);

    // Layout 0 is (x offset, y offset, tick index)
    m_gl.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // Layout 1 is (texture x position, texture y position)
    m_gl.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    m_gl.glEnableVertexAttribArray(0);
    m_gl.glEnableVertexAttribArray(1);

    m_gl.glBindBuffer(GL_ARRAY_BUFFER, 0);
}


/* --------------------------------------------------------------------------------------------------------
	Write text to buffer (used for openGL rendering in the shader)
----------------------------------------------------------------------------------------------------------*/


void AxisTickLabels::yWriteTextToBuffer(const std::vector<std::string>& allTickText, int numDigits)
{
    writeTextToBuffer(allTickText, numDigits, m_yLabelVBO, m_yTickLabelVAO, false);
}


void AxisTickLabels::xWriteTextToBuffer(const std::vector<std::string>& allTickText, int numDigits)
{
    writeTextToBuffer(allTickText, numDigits, m_xLabelVBO, m_xTickLabelVAO, true);
}


void AxisTickLabels::writeTextToBuffer(const std::vector<std::string>& allTickText, int numDigits, unsigned int labelVBO, VertexArrayObject& labelVAO, bool isXAxis)
/*
	Each render loop interation, we need to update the tick labels that are drawn. First, the character
	positions and texture atlas position are written to the VERTEX_ARRAY_BUFFER here (one for x axis, one for y-axis).

	This function is shared between x and y axis, so the VBO identifier axis type must be passed.
*/
{
    std::vector<float> vertices;
    vertices.reserve(numDigits);

    int numTicks = allTickText.size();

    // Cycle through each tick (for each tick, we have a multi-character label)
    for (int tick = 0; tick < numTicks; tick++)
    {
        const std::string& tickLabel = allTickText[tick];

        int charXOffset = 0;  // position of the character in the tick label

        // Compute the length of this tickLabel, used for
        // adjusting position to center / left edge
        int tickLabelWidth = 0;
        {
            for (const char& c : tickLabel)
            {
                Character ch = m_charTextureAtlas.chars().at(c);

                tickLabelWidth += ch.size.x;
            }
        }

        // For each character, compute the offsets and store in the buffer
        for (const char& c : tickLabel)
        {
            Character ch =  m_charTextureAtlas.chars().at(c);

            // Compute alignments that require length of the word at this stage. If
            // we are on the x-axis, shift y position down by 1 numeric glyph  so it
            // lies under the axis, and along by half the label width so it is centered
            // around the tick on the x-axis.
            //
            // For the y-axis, by default the tick starts on the axis, so we want to shift
            // by one tick label length so it ends at the axis.
            float xpos, ypos;
            if (isXAxis)
            {
                xpos = charXOffset + ch.bearing.x - (float)tickLabelWidth / 2;
                ypos = 0 - (ch.size.y - ch.bearing.y) - m_charTextureAtlas.textYSize();
            }
            else
            {
                if (m_configs.m_plotOptions.axisRight)
                {
                    xpos = charXOffset + ch.bearing.x;
                }
                else
                {
                    xpos = charXOffset + ch.bearing.x - (float)tickLabelWidth;
                }
                ypos = 0.0f - (ch.size.y - ch.bearing.y) - (float)m_charTextureAtlas.textYSize() / 2.0f;
            }

            // Compute the position of this characters texture in the texture atlas.
            float texelWidth = 1.0f / (float)m_charTextureAtlas.atlasWidth();
            float texLeft = (float)ch.xTexturePos / (float)m_charTextureAtlas.atlasWidth();;
            float texRight = ((float)ch.xTexturePos + (float)ch.size.x) / (float)m_charTextureAtlas.atlasWidth();;

            texLeft += 0.5 * texelWidth;
            texRight -= 0.5 * texelWidth;

            // Store everything in the buffer to
            // be passed to font_vertex_shader
            //                                x pos world              y pos world             tick             texture x pos (NDC)     texture y pos (NDC)
            vertices.insert(vertices.end(), { xpos,                    ypos,                 (float)tick,       texLeft,                (float)((double)ch.size.y / (double)m_charTextureAtlas.atlasHeight()) });
            vertices.insert(vertices.end(), { xpos,                    ypos + ch.size.y,     (float)tick,       texLeft,                0.0f });
            vertices.insert(vertices.end(), { xpos + ch.size.x,        ypos + ch.size.y,     (float)tick,       texRight,               0.0f });

            vertices.insert(vertices.end(), { xpos,                    ypos,                 (float)tick,       texLeft,                (float)((double)ch.size.y / (double)m_charTextureAtlas.atlasHeight()) });
            vertices.insert(vertices.end(), { xpos + ch.size.x,        ypos + ch.size.y,     (float)tick,       texRight,               0.0f });
            vertices.insert(vertices.end(), { xpos + ch.size.x,        ypos,                 (float)tick,       texRight,               (float)((double)ch.size.y / (double)m_charTextureAtlas.atlasHeight()) });

            charXOffset += (ch.advance >> 6);
        }
    }

    // Store the array in the buffer.
    labelVAO.bind();

    m_gl.glBindBuffer(GL_ARRAY_BUFFER, labelVBO);

    m_gl.glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        vertices.size() * sizeof(float),
        vertices.data()
    );

    m_gl.glBindBuffer(GL_ARRAY_BUFFER, 0);
    labelVAO.unBind();

}


/* --------------------------------------------------------------------------------------------------------
	Write text to buffer (used for openGL rendering in the shader)
----------------------------------------------------------------------------------------------------------*/


void AxisTickLabels::drawYTickLabels(glm::mat4& viewportTransform, float tickStartView, float tickDeltaView, float axisPos, int numTicks, float x, float y, int startAtLowestIdx, double yHeightProportion)
{
	m_yTickLabelVAO.bind();
    draw(viewportTransform, tickStartView, tickDeltaView, axisPos, numTicks, x, y, 0, startAtLowestIdx, yHeightProportion);
	m_yTickLabelVAO.unBind();
}


void AxisTickLabels::drawXTickLabels(glm::mat4& viewportTransform, float tickStartView, float tickDeltaView, float axisPos, int numTicks, float x, float y, int startAtLowestIdx)
{
	m_xTickLabelVAO.bind();
    draw(viewportTransform, tickStartView, tickDeltaView, axisPos, numTicks, x, y, 1, startAtLowestIdx, 0.0f);  // last arg unused
	m_xTickLabelVAO.unBind();
}


void AxisTickLabels::draw(glm::mat4& viewportTransform, float tickStartView, float tickDeltaView, float axisPos, int numChars, float x, float y, int isX, int startAtLowestIdx, double yHeightProportion)
/*
	Draw the axis tick labels.

	1) Generate transformation from world (I guess 'glyph' coordinates)to to NDC space with `projection` matrix

	2) Setup all uniform variables. See `AxesObject.cpp` for details and draw.
*/
{
    m_gl.glEnable(GL_BLEND);
    m_gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // todo: SHOULD REALLY INCORPORATE yAxisProportion into this!¬?
    // Glypys are in pixels, so we can just scale to the number of pixels
    // on our window to get to NDC
    glm::mat4 projection = glm::scale(
        glm::mat4(1.0f),
        glm::vec3(2.0f / m_linkedSubplot.windowViewport().getWindowWidth(),
        2.0f / m_linkedSubplot.windowViewport().getWindowHeight(),
        1.0f)
    );

    m_charTextureAtlas.activateAndBind();

    m_fontProgram.bind();
    m_fontProgram.setUniform1f("axisPos", axisPos);
    m_fontProgram.setUniformMatrix4fc("axes_viewport_transform", viewportTransform);
    m_fontProgram.setUniform1f("startPos", tickStartView);
    m_fontProgram.setUniform1f("tickDelta", tickDeltaView);
    m_fontProgram.setUniformMatrix4fc("projection", projection);
    m_fontProgram.setUniform1i("text", 0);
    m_fontProgram.setUniform1i("isX", isX);
    m_fontProgram.setUniform1i("startAtLowestIdx", startAtLowestIdx);

    if (isX)
    {
        m_fontProgram.setUniform4f("fontColor", m_linkedSubplot.xAxisSettings().fontColor);
        m_fontProgram.setUniform1f("yHeightProportion", 1.0f);
    }
    else
    {
        m_fontProgram.setUniform4f("fontColor", m_linkedSubplot.yAxisSettings().fontColor);
        m_fontProgram.setUniform1f("yHeightProportion", yHeightProportion);
    }
    m_gl.glDrawArrays(GL_TRIANGLES, 0, 6 * numChars);

    m_fontProgram.unBind();
    m_gl.glBindVertexArray(0);
    m_charTextureAtlas.unBind();
    m_gl.glDisable(GL_BLEND);
}




