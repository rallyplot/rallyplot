#version 330 core

layout (location = 0) in vec3 inputPos;
layout (location = 1) in vec2 texturePos;

uniform float xInitWindowScaler;
uniform float yInitWindowScaler;

uniform float yTextTopPad;
uniform float yBoxPad;

uniform float widthPositionOffset;
uniform float heightPositionOffset;

uniform float labelPixelsHeight;
uniform float yInterLinePad;

out vec2 TexCoords;

void main()
{
    float labelIndex = inputPos.z;
    vec2 projInput = vec2(inputPos.x * xInitWindowScaler, inputPos.y * yInitWindowScaler);

    float xStart = 1.0f - widthPositionOffset;
    float yStart = 1.0f - (yBoxPad + yTextTopPad + labelIndex * (labelPixelsHeight + yInterLinePad)) * yInitWindowScaler;

    gl_Position = vec4(projInput.x + xStart, projInput.y + yStart, 0.0, 1.0f);

    TexCoords = texturePos.xy;

}
