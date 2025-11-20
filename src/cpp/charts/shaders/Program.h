#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include <glm.hpp>


class Program
{
public:
    Program(std::string vertexShaderPath, std::string fragmentShaderPath, std::string geometryShaderPath, QOpenGLFunctions_3_3_Core& glFunctions);
    Program(std::string vertexShaderPath, std::string fragmentShaderPath, QOpenGLFunctions_3_3_Core& glFunctions);
	~Program();

	void setupAndBindProgram();
	void teardownProgram();

	void bind();
	void unBind();

	void setUniform4f(const std::string& uniformName, glm::vec4 value);
	void setUniform3f(const std::string& uniformName, glm::vec3 value);
	void setUniform1i(const std::string& uniformName, int value);
	void setUniformMatrix4fc(const std::string& uniformName, glm::mat4 value);
	void setUniform1f(const std::string& uniformName, float value);

	unsigned int getId() const { return m_programID; };

private:
	unsigned int m_programID = 0;
	std::string m_vertexShaderPath;
	std::string m_fragmentShaderPath;
    std::string m_geometryShaderPath = "";  // TODO: probably a better way to do this rather than overloading!!
    QOpenGLFunctions_3_3_Core& m_gl;
};
