#version 330 core

layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

// TOOD: no aspect ratio or yPosition correction here!

uniform float width;
uniform float aspectRatio;
uniform float yHeightProportion;  // TODO currently unused, is already accounted for some reason...
uniform float subplotHeightProportion;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vec2 p0 = vec2(gl_in[0].gl_Position.xy);
    vec2 p1 = vec2(gl_in[1].gl_Position.xy);

    vec2 segVec = vec2(p1 - p0);

    vec2 norm = normalize(vec2(-segVec.y, segVec.x));

    float halfWidth = width * 0.5 * 2.0;

    vec2 scaledNorm = vec2(norm.x * halfWidth * (1.0f / aspectRatio), norm.y * halfWidth) * (1.0f / yHeightProportion) * (1.0f / subplotHeightProportion);  //  * halfWidth * aspectRatio);  //  * aspectRatio

    gl_Position = vec4(p0 + scaledNorm, 0.0f, 1.0f);
    EmitVertex();

    gl_Position = vec4(p0 - scaledNorm, 0.0f, 1.0f);
    EmitVertex();

    gl_Position = vec4(p1 + scaledNorm, 0.0f, 1.0f);
    EmitVertex();

    gl_Position = vec4(p1 - scaledNorm, 0.0f, 1.0f);
    EmitVertex();

    EndPrimitive();
}
