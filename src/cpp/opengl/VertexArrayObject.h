#pragma once

#include <QOpenGLFunctions_3_3_Core>

class VertexArrayObject
/*
	Convenience class to handle Vertex Array Objects. Provides
	a simple wrapper for initialisation, binding and unbinding.

	To use, initialise the glass and setup the VAO. We do not want
	to do this by default as in practice it can be useful to initialise
	then setup later.Then, bind the approrpiate VBO.

    Delete copy and move constructors to avoid improper handling
    of underlying resources.
*/
{
public:
    VertexArrayObject(QOpenGLFunctions_3_3_Core& glFunctions);
	~VertexArrayObject();

    VertexArrayObject(const VertexArrayObject&) = delete;
    VertexArrayObject& operator=(const VertexArrayObject&) = delete;
    VertexArrayObject(VertexArrayObject&&) = delete;
    VertexArrayObject& operator=(VertexArrayObject&&) = delete;

	void bind();
	void unBind();

	void setup();

	unsigned int getVAO() const;

private:
    QOpenGLFunctions_3_3_Core& m_gl;

	unsigned int m_VAO;
};
