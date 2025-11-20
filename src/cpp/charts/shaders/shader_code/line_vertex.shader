#version 330 core

layout(location = 0) in float data;

uniform float xDelta;

uniform mat4 NDCMatrix;
uniform float offset;

flat out int vIndex;

// Dummies, used for alignment with candlestick_line_shader
// and in future, Color could be used for setting dynamic colors
out vec4 Color;
out vec4 FragPos;
uniform vec4 color;

void main()
{
    float xPosCenter = xDelta * gl_VertexID;

    float xPos = xPosCenter - offset;
    float yPos = data;

    gl_Position = NDCMatrix * vec4(xPos, yPos, 0.0f, 1.0);

    vIndex = gl_VertexID;

    Color = color;  // TODO: this is super wasteful
}
