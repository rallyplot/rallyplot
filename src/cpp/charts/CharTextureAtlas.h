#ifndef CHARTEXTUREATLAS_H
#define CHARTEXTUREATLAS_H

#include <glm.hpp>
#include <qfileinfo.h>
#include <qopenglfunctions_3_3_core.h>
#include "../include/Plotter.h"

struct Character {
    unsigned int textureID;   // ID handle of the glyph texture
    glm::ivec2 size;          // Size of glyph
    glm::ivec2 bearing;       // Offset from baseline to left/top of glyph
    int advance;              // Offset to advance to next glyph
    int xTexturePos;          // Offset in texture atlas (x)
};


class CharTextureAtlas
/*
    Base texture atlas containing 126 characters (.ttf) of the fontName and fontSize
    provided. fontName must match a font within the /fonts directory.

    Multiple character atlases are used in order to allow different font sizes (e.g.
    for axis labels vs. legend). A unique GL_TEXTURE<number> must be passed. Currently
    used texture bindings are:

    0 : axis labels (CharTextureAtlast)
    1 : scatterplot textures
    2 : legend text (CharTextureAtlast)
 */
{
public:

    CharTextureAtlas(
        QOpenGLFunctions_3_3_Core& glFunctions,
        GLenum glTextureId,
        Font font,
        int fontSize
        );
    ~CharTextureAtlas();

    const std::map<char, Character>& chars() { return m_characters; };
    unsigned int atlasHeight() { return m_atlasHeight; };
    unsigned int atlasWidth() { return m_atlasWidth; };
    int textYSize() {return m_numericTextYSize; };
    Character getCharacter(const char& char_);
    void activateAndBind();
    void unBind();

private:

    QOpenGLFunctions_3_3_Core& m_gl;
    GLenum m_GL_TEXTURE;
    std::string m_fontName;
    int m_fontSize;

    unsigned int m_atlas;
    std::map<char, Character> m_characters;
    unsigned int m_atlasWidth = 0;
    unsigned int m_atlasHeight = 0;
    int m_numericTextYSize;

    void setupCharTextureAtlas();
};

#endif // CHARTEXTUREATLAS_H
