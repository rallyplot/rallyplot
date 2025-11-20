#include "VertexArrayObject.h"


VertexArrayObject::VertexArrayObject(QOpenGLFunctions_3_3_Core& glFunctions)
    : m_VAO(0),
      m_gl(glFunctions)
{
};


VertexArrayObject::~VertexArrayObject()
{

    if (m_VAO != 0)
    {
        m_gl.glDeleteVertexArrays(1, &m_VAO);
    }
}


void VertexArrayObject::setup()
{
    m_gl.glGenVertexArrays(1, &m_VAO);

    m_gl.glBindVertexArray(m_VAO);
}


void VertexArrayObject::bind()
{
    m_gl.glBindVertexArray(m_VAO);
}


void VertexArrayObject::unBind()
{
    m_gl.glBindVertexArray(0);
}


unsigned int VertexArrayObject::getVAO() const
{
    return m_VAO;
}
