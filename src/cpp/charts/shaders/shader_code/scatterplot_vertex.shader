#version 330 core
/*

*/
layout(location = 0) in vec2 data;
layout(location = 1) in vec2 quadOffset;

uniform float xDelta;

uniform mat4 NDCMatrix;
uniform float offset;

uniform int fixedSize;
uniform float markerSizeFixed;
uniform float markerSizeFree;
uniform float aspectRatio;

out vec4 FragPos;
out vec2 texCoords;

void main()
{
    float xOffset, yOffset;

    if (fixedSize == 1)
    {
        float size = markerSizeFixed;
        xOffset = (quadOffset.x < 0.0) ? -markerSizeFixed :  markerSizeFixed;
        yOffset = (quadOffset.y < 0.0) ? -markerSizeFixed :  markerSizeFixed;
    }
    else
    {
        vec4 calcXOffset = NDCMatrix * vec4(quadOffset.x * xDelta * markerSizeFree, 0.0f, 0.0f, 0.0f);
        vec4 calcYOffset = NDCMatrix * vec4(quadOffset.y * xDelta * markerSizeFree, 0.0f, 0.0f, 0.0f);

        xOffset = calcXOffset.x;
        yOffset = calcYOffset.x;
    }

    yOffset *= aspectRatio;

    float xPos = data.x - offset;
    float yPos = data.y;

    gl_Position = NDCMatrix * vec4(xPos, yPos, 0.0f, 1.0f);

    gl_Position.x = gl_Position.x + xOffset;
    gl_Position.y = gl_Position.y + yOffset;

    FragPos = gl_Position;

    texCoords = (quadOffset + 1.0f) * 0.5f;
}

