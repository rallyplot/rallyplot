#version 330 core
/*
    see CandleStickPlot.draw() for uniforms
*/
layout(location = 0) in vec2 BasisVertex;

layout(location = 1) in vec2 OpenClose;   // (Open, Close)
layout(location = 2) in vec2 lowHigh;      // (Low, High)


uniform float xDelta;

uniform int drawMode;

uniform mat4 NDCMatrix;
uniform float offset;

uniform vec4 upColor;
uniform vec4 downColor;

uniform float candleWidthRatio;
uniform float capWidthRatio;

out vec4 Color;
out vec4 FragPos;

flat out int vIndex;

void main()
{
        float xPosCenter;
        if (drawMode == 4 || drawMode == 5)
        {
            xPosCenter = xDelta * gl_VertexID;
        }
        else
        {
            xPosCenter = xDelta * gl_InstanceID;
        }
        float candleWidth = xDelta * candleWidthRatio;
        float capWidth = candleWidth * capWidthRatio;

        float open = OpenClose.x;
        float close = OpenClose.y;

        bool goingUp = (close > open);

        float yPos;
        float xPos;

        // Candle body or vertical line extending open / close
        if (drawMode == 0 || drawMode == 3)
        {
            if (goingUp)
            {
                yPos = (close - open) * BasisVertex.y + close;
            }
            else
            {
                yPos = (open - close) * BasisVertex.y + open;
            }
            xPos = xPosCenter - offset - (candleWidth / 2.0f) + (BasisVertex.x * candleWidth);

            if (drawMode == 3)
            {
                xPos += candleWidth * 0.5;
            }
        }

        // line extending to low / high
        else if (drawMode == 1 || drawMode == 2)
        {
             yPos = (lowHigh.y - lowHigh.x) * BasisVertex.y + lowHigh.y;
             xPos = xPosCenter - offset + (BasisVertex.x * capWidth);
        }

        // line only (open: 4) or (close: 5)
        else if (drawMode == 4 || drawMode == 5)
        {
            xPos = xPosCenter - offset;
            yPos = (drawMode == 4) ? open : close;
        }

        if (goingUp)
        {
            Color = upColor;
        }
        else
        {
            Color = downColor;
        }

        gl_Position = NDCMatrix * vec4(xPos, yPos, 0.0f, 1.0);  // TODO: RENAME!

        FragPos = gl_Position;

        vIndex = gl_VertexID;
}

