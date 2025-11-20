#version 330 core

layout(location = 0) in vec2 data;

uniform mat4 NDCMatrix;
uniform float offset;  // TODO: why does line plot not use the offset?

void main()
{
 //   float xPos = data.x; // - offset;
   // float yPos = data.y;
    float xPos = data.x - offset;
    float yPos = data.y;
    gl_Position = NDCMatrix * vec4(xPos, yPos, 0.0f, 1.0f);  // NDCMatrix *
}

