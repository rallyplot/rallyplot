#pragma once

#include <ft2build.h>
#include <qfiledevice.h>
#include <qfileinfo.h>
#include FT_FREETYPE_H

#include <vector>

#include <QOpenGLFunctions_3_3_Core>

#include "shaders/Program.h"
#include "../opengl/VertexArrayObject.h"
#include "shaders/Program.h"
#include "../Configs.h"
#include "CharTextureAtlas.h"


class LinkedSubplot; // forward declaration


class AxisTickLabels
/*
	Class to manage a texture atlas of TrueType chracter gylphs
	and buffers used to draw text labels. The procedure is:

	1) One-time setup of the atlas of all gylphs held in a single row texture

	2) When time to draw tick labels, a vector of characters to draw is passed to
	   writeTextToBuffer functions. This stores information on the characters to draw including
	   x, y positioning offsets, as well as position in the texture atlas.

	3) The shader reads these buffers in and positions the characters appropriately,
	   rendering them based on their positioning in the texture atlas.
*/
{
public:

    AxisTickLabels(Configs& configs, LinkedSubplot& subplot, QOpenGLFunctions_3_3_Core& glFunctions);
	~AxisTickLabels();

    AxisTickLabels(const AxisTickLabels&) = delete;
    AxisTickLabels& operator=(const AxisTickLabels&) = delete;
    AxisTickLabels(AxisTickLabels&&) = delete;
    AxisTickLabels& operator=(AxisTickLabels&&) = delete;

	void setupCharAtlasTexture();

	void xSetupVAO();
	void ySetupVAO();

    void yWriteTextToBuffer(const std::vector<std::string>& allTickText, int numDigits);
    void xWriteTextToBuffer(const std::vector<std::string>& allTickText, int numDigits);

    void drawYTickLabels(
        glm::mat4& viewportTransform,
        float tickStartView,
        float tickDeltaView,
        float axisPos,
        int numChars,
        float x,
        float y,
        int startAtLowestIdx,
        double yHeightProportion
    );

    void drawXTickLabels(
        glm::mat4& viewportTransform,
        float tickStartView,
        float tickDeltaView,
        float axisPos,
        int numChars,
        float x,
        float y,
        int startAtLowestIdx
    );

private:

    Configs& m_configs;
    LinkedSubplot& m_linkedSubplot;
    QOpenGLFunctions_3_3_Core& m_gl;
    Program m_fontProgram;
    VertexArrayObject m_xTickLabelVAO;
    VertexArrayObject m_yTickLabelVAO;
    CharTextureAtlas m_charTextureAtlas;

	void setupVAO(unsigned int& VBO, std::vector<float>& fontBuffer);

	unsigned int m_yLabelVBO;
	unsigned int m_xLabelVBO;

	// Make an array that we will never use all of.
    // 7 elements per vertex, 6 vertices per letter, max 10
   // letters per tick label, max 100 (well max) labels per axis.
    int m_maxEverBufferLength = 7 * 6 * 10 * cfg_MAX_NUM_TICKS;
	std::vector<float> m_yLabelBuffer = std::vector<float>(m_maxEverBufferLength, 1.0f);
	std::vector<float> m_xLabelBuffer = std::vector<float>(m_maxEverBufferLength, 1.0f);

	// Internal writers and drawers
    void writeTextToBuffer(
        const std::vector<std::string>& allTickText,
        int numDigits,
        unsigned int labelVBO,
        VertexArrayObject& labelVAO,
        bool isXAxis
    );

	void draw(
		glm::mat4& viewportTransform,
		float tickStartView,
		float tickDeltaView,
		float axisPos,
		int numChars,
		float x,
		float y,
		int isX, 
        int startAtLowestIdx,
        double yHeightProportion
	);
};
