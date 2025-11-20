#include <iostream>
#include <gtc/type_ptr.hpp>
#include "Program.h"
#include "Shaders.h"


Program::Program(std::string vertexShaderPath, std::string fragmentShaderPath, std::string geometryShaderPath, QOpenGLFunctions_3_3_Core& glFunctions)
    : m_vertexShaderPath(vertexShaderPath), m_fragmentShaderPath(fragmentShaderPath),  m_gl(glFunctions)
{
    m_geometryShaderPath = geometryShaderPath;
};


Program::Program(std::string vertexShaderPath, std::string fragmentShaderPath, QOpenGLFunctions_3_3_Core& glFunctions)
    : m_vertexShaderPath(vertexShaderPath), m_fragmentShaderPath(fragmentShaderPath), m_gl(glFunctions)
{};


Program::~Program()
{
	teardownProgram();
}


void Program::setupAndBindProgram()
// Setup a program by linking together vertex and fragment shaders.
// Note this links the inputs / outputs, so output of vertex shader
// will be passed to inputs of fragment shader.
{
	// Initialise the shader objects and compile the shaders
    Shader vshad(GL_VERTEX_SHADER, m_vertexShaderPath, m_gl);
    Shader fshad(GL_FRAGMENT_SHADER, m_fragmentShaderPath, m_gl);

	vshad.compileShader();
	fshad.compileShader();

	// Create thre program and link the shaders to it.
    m_programID = m_gl.glCreateProgram();

    m_gl.glAttachShader(m_programID, vshad.getId());
    m_gl.glAttachShader(m_programID, fshad.getId());


    bool hasGeometryShader = m_geometryShaderPath != "";

    if (hasGeometryShader)
    {
        Shader gshad(GL_GEOMETRY_SHADER, m_geometryShaderPath, m_gl);
        gshad.compileShader();
        m_gl.glAttachShader(m_programID, gshad.getId());

        m_gl.glLinkProgram(m_programID);
        gshad.deleteShader();
    }
    else
    {
        m_gl.glLinkProgram(m_programID);
    }

	vshad.deleteShader(); 
	fshad.deleteShader();

    // Check the shaders were linked sucessfully
	int sucess;
    m_gl.glGetProgramiv(m_programID, GL_LINK_STATUS, &sucess);
	char infoLog[512];
	if (!sucess)
	{
        m_gl.glGetProgramInfoLog(m_programID, 512, NULL, infoLog);
		std::cout << "ERROR IN PROGRAM LINKING: " << infoLog << std::endl;
		return;
	}

	bind();
}


void Program::teardownProgram()
{
	if (m_programID != 0)
	{
        m_gl.glDeleteProgram(m_programID);
		m_programID = 0;
	}
}


void Program::bind()
{
    m_gl.glUseProgram(m_programID);
}


void Program::unBind()
{
    m_gl.glUseProgram(0);
}


void Program::setUniform4f(const std::string& uniformName, glm::vec4 value)  // TODO: factor out glGetUniformLocation like cherno!
{
    int location = m_gl.glGetUniformLocation(m_programID, uniformName.c_str());
    m_gl.glUniform4f(location, value[0], value[1], value[2], value[3]);
}


void Program::setUniform3f(const std::string& uniformName, glm::vec3 value)
{
    int location = m_gl.glGetUniformLocation(m_programID, uniformName.c_str());
    m_gl.glUniform3f(location, value[0], value[1], value[2]);
}


void Program::setUniform1i(const std::string& uniformName, int value)
{
    int location = m_gl.glGetUniformLocation(m_programID, uniformName.c_str());
    m_gl.glUniform1i(location, value);
}


void Program::setUniform1f(const std::string& uniformName, float value)
{
    int location = m_gl.glGetUniformLocation(m_programID, uniformName.c_str());
    m_gl.glUniform1f(location, value);
}


void Program::setUniformMatrix4fc(const std::string& uniformName, glm::mat4 value)
// requires pointer to ensure glm matrix is in form openGL can handle.
{
    int location = m_gl.glGetUniformLocation(m_programID, uniformName.c_str());
    m_gl.glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
;}
