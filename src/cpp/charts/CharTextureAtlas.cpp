#include "../include/Plotter.h"
#include "../Utils.h"
#include "CharTextureAtlas.h"
#include "freetype/freetype.h"
#include <iostream>


CharTextureAtlas::CharTextureAtlas(
    QOpenGLFunctions_3_3_Core& glFunctions,
    GLenum glTextureId,
    Font font,
    int fontSize
    ) : m_gl(glFunctions),
    m_GL_TEXTURE(glTextureId),
    m_fontName(utils_fontEnumToStr(font)),
    m_fontSize(fontSize)
{
    setupCharTextureAtlas();
};


CharTextureAtlas::~CharTextureAtlas()
{
    m_gl.glDeleteTextures(1, &m_atlas);
    m_atlas = 0;
};


void CharTextureAtlas::setupCharTextureAtlas()
/*
    Setup a texture atlas that includes all characters in a single row.

    The key features of each char are stored in `m_characters`.
    Sizes are in pixels.
*/
{
    // Initialise the freetype library
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        // Use `cerr` here so we don't need to worry about resource leaks.
        std::cerr << "CRITICAL ERROR: Freetype could not init FreeType Library" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Load the fonts
    QString qrcFilename = QString::fromStdString(":/fonts/charts/fonts/" +  m_fontName + ".ttf");

    QFile file(qrcFilename);

    if (!file.open(QIODevice::ReadOnly))
    {
        std::cerr << "CRITICAL ERROR: Freetype failed to open font from resources!" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    QByteArray fontData = file.readAll();
    file.close();

    FT_Face face;
    if (FT_New_Memory_Face(ft, (const FT_Byte*)fontData.constData(), fontData.size(), 0, &face))
    {
        std::cerr << "CRITICAL ERROR: Freetype failed to load font" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    FT_Set_Pixel_Sizes(face, 0, static_cast<FT_UInt>(m_fontSize));

    // Find the total height and width that the character atlas will be
    m_atlasWidth = 0;
    m_atlasHeight = 0;
    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "CRITICAL ERROR: Freetype failed to load Glyph" << std::endl;
            std::exit(EXIT_FAILURE);
        }
        m_atlasWidth += face->glyph->bitmap.width;
        m_atlasHeight = std::max(m_atlasHeight, face->glyph->bitmap.rows);

    }

    // Setup the texture in which the atlas will be bound
    m_gl.glActiveTexture(m_GL_TEXTURE);
    m_gl.glGenTextures(1, &m_atlas);
    m_gl.glBindTexture(GL_TEXTURE_2D, m_atlas);
    m_gl.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    m_gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_atlasWidth, m_atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

    // Fill the texture with all characters
    int xTexturePos = 0;
    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "CRITICAL ERROR: Freetype failed to load Glyph" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        m_gl.glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            xTexturePos,
            0,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
            );

        Character character = {
            m_atlas,											// ID handle of the glyph texture
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),	// Size of glyph
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),		// Offset from baseline to left/top of glyph (bearing)
            (int)face->glyph->advance.x,												// Offset to advance to for next character
            xTexturePos,

        };
        m_characters.insert(std::pair<char, Character>(c, character));

        xTexturePos += face->glyph->bitmap.width;
    }

    // Store the size of a numeric character which is used later
    if (FT_Load_Char(face, '1', FT_LOAD_RENDER))
    {
        std::cerr << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    m_numericTextYSize = face->glyph->bitmap.rows;

    // Set the texture configs
    m_gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Important FreeType cleanup
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // this must be turned off for Qt QPainter to work
    // properly over OpenGl widget.
    m_gl.glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}


void CharTextureAtlas::activateAndBind()
{
    m_gl.glActiveTexture(m_GL_TEXTURE);
    m_gl.glBindTexture(GL_TEXTURE_2D, m_atlas);
};

void CharTextureAtlas::unBind()
{
    m_gl.glBindTexture(GL_TEXTURE_2D, 0);
};


Character CharTextureAtlas::getCharacter(const char& char_)
{
    try
    {
        return this->chars().at(char_);
    }
    catch (const std::out_of_range& e)
    {
        std::cerr << "Cannot find the character '" + std::string(1, char_) + "' in the atlas." << std::endl;
    }
}
