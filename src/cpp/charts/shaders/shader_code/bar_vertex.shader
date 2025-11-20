#version 330 core
/*
*/
layout(location = 0) in float yData;
layout(location = 1) in vec2 barVertex;

uniform float xDelta;
uniform float minValue;

uniform int drawMode;

uniform mat4 NDCMatrix;
uniform float offset;

uniform float widthRatio;


void main()
{
        float xPosCenter = xDelta * gl_InstanceID;

        float yPos = yData * barVertex.y  + minValue;

        float barWidth = xDelta * widthRatio;

        float xPos;
        if (drawMode == 0)
        {
            xPos = xPosCenter - offset - (barWidth / 2.0f) + (barVertex.x * barWidth);
        }
        else
        {
            xPos = xPosCenter - offset - (barWidth / 2.0f);
        }
        gl_Position = NDCMatrix * vec4(xPos, yPos, 0.0f, 1.0);
}

