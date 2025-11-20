#version 330 core

layout(location = 0) in vec2 inputPos;
layout(location = 1) in int LeftOrRight;

uniform float boxWidth;
uniform float boxHeight;

uniform vec4 leftColor;
uniform vec4 rightColor;

uniform float widthPositionOffset;
uniform float heightPositionOffset;

out vec4 Color;


void main()
{
    vec2 scaledBox = vec2(inputPos.x * boxWidth, inputPos.y * boxHeight);

    float xStart = 1.0f - widthPositionOffset;
    float yStart = 1.0f - heightPositionOffset;

    gl_Position = vec4(scaledBox.x + xStart, scaledBox.y + yStart, 0.0, 1.0f);

    if (LeftOrRight == 0)
    {
        Color = leftColor;
    }
    else
    {
        Color = rightColor;
    }
}

