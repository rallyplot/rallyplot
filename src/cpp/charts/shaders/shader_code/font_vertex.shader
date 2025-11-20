#version 330 core

layout (location = 0) in vec3 worldPos;
layout (location = 1) in vec2 texturePos;

out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 axes_viewport_transform;
uniform float axisPos;
uniform float startPos;
uniform float tickDelta;
uniform int isX;
uniform int startAtLowestIdx;
uniform float yHeightProportion;


void main()
{
    float tickIndex = worldPos.z;

    vec4 NDCProj = projection * vec4(worldPos.x, worldPos.y * (1.0f / yHeightProportion), 0.0f, 1.0f);  // TODO: worldPos (here and in AxisTickLabels isn't world its like glyph

    if (isX == 1)  
    {
        float startPosNDC;
        if (startAtLowestIdx == 0)
            startPosNDC = startPos - tickIndex * tickDelta;  
        else
            startPosNDC = startPos + tickIndex * tickDelta;

        gl_Position = axes_viewport_transform * vec4(startPosNDC + NDCProj.x, axisPos + NDCProj.y, 0.0f, 1.0f);
    }    
    else
        {
            float startPosNDC = startPos + tickIndex * tickDelta;
            gl_Position = axes_viewport_transform * vec4(axisPos + NDCProj.x, startPosNDC + NDCProj.y, 0.0f, 1.0f);
        }

    TexCoords = texturePos.xy;
}
