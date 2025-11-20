#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <QFile>
#include <QString>
#include "Shaders.h"


Shader::Shader(GLenum shaderType, const std::string& filepath, QOpenGLFunctions_3_3_Core& glFunctions)
    : m_shaderType(shaderType), m_filepath(filepath), m_gl(glFunctions), m_id(0)
{
}


Shader::~Shader()
{
    deleteShader();
}


void Shader::compileShader()
{
    QString qrcFilename = QString::fromStdString(":/shaders/charts/shaders/shader_code/" + m_filepath);

    QFile file(qrcFilename);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        std::cerr << "CRITICAL ERROR: Failed to open shader from QFile (qrc)." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    QTextStream in(&file);
    QString fileContent = in.readAll();
    std::string shaderSource = fileContent.toStdString();

    const GLchar* shaderSourcePtr = shaderSource.c_str();

	// create an empty shader object and return nonzero reference
    m_id = m_gl.glCreateShader(m_shaderType);

	// Set the source code in the shader object to that
    // specified by `shaderSourcePtr`.
    m_gl.glShaderSource(m_id, 1, &shaderSourcePtr, NULL);

    m_gl.glCompileShader(m_id);

	// Check if the shader compiled correctly.
	int sucess;
	char infoLog[512];
    m_gl.glGetShaderiv(m_id, GL_COMPILE_STATUS, &sucess);

	if (!sucess)
	{
        m_gl.glGetShaderInfoLog(m_id, 512, NULL, infoLog);
        std::cerr << "CRITICAL ERROR: shader " << m_filepath << "compilation failed. " << infoLog << std::endl;
        std::exit(EXIT_FAILURE);
	}
}


void Shader::deleteShader()
{
    if (m_id != 0)
	{
        m_gl.glDeleteShader(m_id);
        m_id = 0;
	}
}



std::string Shader::readShaderFile(const std::string& filepath)
// Read from file into a string variable and return.
{
	std::ifstream file(filepath);

	if (!file.is_open())
	{
        std::cerr << "CRITICAL ERROR: Failed to open shader at: " << filepath << std::endl;
        std::exit(EXIT_FAILURE);
	}

	std::stringstream buffer;
	buffer << file.rdbuf();

	std::string source = buffer.str();

	file.close();

	return source;
}
