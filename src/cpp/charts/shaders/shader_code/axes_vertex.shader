#version 330 core

layout (location = 0) in vec2 aPos;

uniform mat4 axes_viewport_transform;

void main()
{
    gl_Position = axes_viewport_transform  * vec4(aPos, 0.0f, 1.0f);
}
