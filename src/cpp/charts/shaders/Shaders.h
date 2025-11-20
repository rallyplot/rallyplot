#pragma once

#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions_3_3_Core>

class Shader
/*
    Class to handle instantiation of shaders.
    Delete move and copy constructors to ensure
    proper handlign of resoruces.
 */
{

public:
    Shader(GLenum shaderType, const std::string& filepath, QOpenGLFunctions_3_3_Core& glFunctions);
	virtual ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&&) = delete;
    Shader& operator=(Shader&&) = delete;

	void compileShader();
	void deleteShader();

    unsigned int getId() const { return m_id; }

private:
	std::string readShaderFile(const std::string& filepath);
    QOpenGLFunctions_3_3_Core& m_gl;

    unsigned int m_id;

	std::string m_filepath;

	GLenum m_shaderType;
};
