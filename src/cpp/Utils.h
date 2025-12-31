#pragma once

#include <cassert>
#include <glm.hpp>
#include <QLabel>
#include <iostream>
#include <qobject.h>
#include <QFontDatabase>
#include <QFont>
#include <QWidget>
#include "include/Plotter.h"


template<typename T>
int utils_getSign(T value)
{
    if (value > 0) return 1;
    if (value < 0) return -1;
    return 0; // Covers both 0 and -0
}


inline QString utils_glmBackgroundColorToQtStylesheet(glm::vec4 color)
/*
    Convert a glm::vec4 to a Qt color stylesheet format.
*/
{
    std::string stylesheet = (
        "background-color: rgba("
        + std::to_string(color[0] * 255) + ", "
        + std::to_string(color[1] * 255) + ", "
        + std::to_string(color[2] * 255) + ", "
        + std::to_string(color[3] * 255) + ");"
        );

    return QString::fromStdString(stylesheet);
}


inline QFont::Weight utils_stringToQtFontWeight(const QString& weightStr)
{
    static const QMap<QString, QFont::Weight> weightMap = {
        {"thin",        QFont::Thin},
        {"extralight",  QFont::ExtraLight},
        {"light",       QFont::Light},
        {"normal",      QFont::Normal},
        {"regular",     QFont::Normal},  // alias
        {"medium",      QFont::Medium},
        {"demibold",    QFont::DemiBold},
        {"bold",        QFont::Bold},
        {"extrabold",   QFont::ExtraBold},
        {"black",       QFont::Black}
    };

    QString key = weightStr.trimmed().toLower();

    if (!weightMap.contains(key))
    {
        std::cerr << "CRITICAL ERROR: Invalid font weight: " << weightStr.toStdString() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    return weightMap.value(key);
}


inline glm::vec4 utils_colorToGlmVec(const float* ptr, int size)
/*
    We allow color to be passed as std::vector of length 1-4 in range [0, 1].
    The RGBA glm::vec4 vector will be filled from left-to-right with available std::vector contents.
 */
{
    for (int i = 0; i < size; i++)
    {
        if (*(ptr + i) < 0.0f || *(ptr + i) > 1.0f)
        {
            std::cerr << "CRITICAL_ERROR: RGBA color must be in the range [0, 1]" << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    glm::vec4 vec;
    if (size == 1)
    {
        vec = glm::vec4(ptr[0], 0.0f, 0.0f, 1.0f);
    }
    else if (size == 2)
    {
        vec = glm::vec4(ptr[0], ptr[1], 0.0f, 1.0f);
    }
    else if (size == 3)
    {
        vec = glm::vec4(ptr[0], ptr[1], ptr[2], 1.0f);
    }
    else if (size == 4)
    {
        vec = glm::vec4(ptr[0], ptr[1], ptr[2], ptr[3]);
    }
    else
    {
        std::cerr << "CRITICAL ERROR: Color must be an array of size 1-4 (that will fill RGBA array from left to right)." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return vec;
}



inline std::string utils_fontEnumToStr(Font font)
{
    if (font == Font::arial)
    {
        return "arial";
    }
    else if (font == Font::consola)
    {
        return "consola";
    }
    else
    {
        throw std::invalid_argument("Invalid font.");
    }
}


inline std::string utils_scatterShapeEnumToStr(ScatterShape shape)
{
    if (shape == ScatterShape::circle)
    {
        return "circle";
    }
    else if (shape == ScatterShape::triangleUp)
    {
        return "triangleUp";
    }
    else if (shape == ScatterShape::triangleDown)
    {
        return "triangleDown";
    }
    else if (shape == ScatterShape::cross)
    {
        return "cross";
    }
    else
    {
        throw std::invalid_argument("Invalid candlestick mode.");
    }
}


inline QFont utils_getQtFont(Font font)
/*
   Load the requested font from the vendored ttf fonts. See `resources.qrc`.
   For now, restrict the font so that it must match ones available for OpenGL.
 */
{
    QString fontFamily = QString::fromStdString(utils_fontEnumToStr(font));

    if (!(font == Font::arial || font == Font::consola))
    {
        std::cerr << "CRITICAL ERROR: `font` must be 'arial' or 'consola'" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (!QFontDatabase::families().contains(fontFamily))
    {
        QString fontPath = QString(":/fonts/charts/fonts/%1.ttf").arg(fontFamily);
        QFontDatabase::addApplicationFont(fontPath);
    }

    return QFont(fontFamily);
}
