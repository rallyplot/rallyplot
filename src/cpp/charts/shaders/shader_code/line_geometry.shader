/*
    This geometry shader implements miter joints for the line plot.
    Using GL_LINE_STRIP_ADJACENCY the points before and after the two
    points that make the line segment are provided.

    To compute the miter joints at each end of the segment, the tangent / normal
    at the joint is computed. The length of the miter (along the normal) is computed
    as the dot(normal, segmend end).

    The vectors are first transformed to incorporate the aspect ratio, as this is
    effected by the angle of the joints. Also, the width is adjusted with
    yHeightProportion so that linewidth is uniform across subplots.

    Because as the joint angle -> 0, miter length -> infinity, there needs
    to be a hard cutoff after which a basic bevel joint is used. The related
    constants (miterLimit, length of miter) and cosLimit (angle between vectors)
    acheive this. The constants are based on visual inspection. The reason there
    are two approaches is because when zooming, the angle and miter length changes
    a lot and having both (slightly redundant) methods is more robust.
*/
#version 330 core
layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 12) out;


uniform float width;
uniform mat4 NDCMatrix;
uniform int useColor;

in vec4 Color[];
in vec4 FragPos[];  // unused, hack for alignment with candlestick.

out vec4 gColor;

uniform float miterLimit;
uniform float aspectRatio;
uniform float yHeightProportion;
uniform float subplotHeightProportion;
uniform int numVertices;

float correctedWidth = width * (1.0 / yHeightProportion) * (1.0f / subplotHeightProportion);
float userMiterLimit = miterLimit;  // TODO: need to scale this by zoom!
float maxLen = correctedWidth * 0.5 * userMiterLimit;
float cosLimit = 1.0;
float minMiterLenDivisor = 0.000000001f;


flat in int vIndex[];

vec2 aspectCorrected(vec2 v) {
    return vec2(v.x * aspectRatio, v.y);
}

vec2 uncorrectAspect(vec2 v) {
    return vec2(v.x / aspectRatio, v.y);
}


void emitQuad(vec2 n0, vec2 n1, vec2 n2, vec2 p0, vec2 p1)
// Emit a quad that is the line segment with corrected
// miter joints at each end
{
    // The sum of the normal vectors to the segment vectors
    // gives the angle bisector at their joint.
    vec2 miter0 = normalize(n0 + n1);

    float cos0 = abs(dot(miter0, vec2(-n1.y, n1.x)));
    float len0 = correctedWidth * 0.5 / max(dot(miter0, n1), minMiterLenDivisor);

    if (len0 > maxLen|| cos0 > cosLimit) {
        miter0 = n1;
        len0 = correctedWidth * 0.5;
    }
    vec2 offset0 = uncorrectAspect(miter0 * len0);

    // Miter at p1
    vec2 miter1 = normalize(n1 + n2);
    vec2 miter1Save = miter1; // TODO

    float cos1 = abs(dot(miter1, vec2(-n1.y, n1.x)));
    float len1 = correctedWidth * 0.5 / max(dot(miter1, n1), minMiterLenDivisor);

    if (len1 > maxLen || cos1 > cosLimit) {
        miter1 = n1;
        len1 = correctedWidth * 0.5;
    }
    vec2 offset1 = uncorrectAspect(miter1 * len1);

    // Emit triangle strip quad in screen space
    gl_Position = vec4(p0 + offset0, 0.0, 1.0);
    if (useColor == 1)
    {
        gColor = Color[1];
    }
       EmitVertex();

    gl_Position = vec4(p0 - offset0, 0.0, 1.0);
    if (useColor == 1)
    {
        gColor = Color[1];
    }
    EmitVertex();

    gl_Position = vec4(p1 + offset1, 0.0, 1.0);
    if (useColor == 1)
    {
        gColor = Color[2];
    }
    EmitVertex();

    gl_Position = vec4(p1 - offset1, 0.0, 1.0);
    if (useColor == 1)
    {
        gColor = Color[2];
    }
    EmitVertex();

    EndPrimitive();
}

void main() {

    // Input positions in clip space (NDC)
    vec2 pPrev = gl_in[0].gl_Position.xy;
    vec2 p0    = gl_in[1].gl_Position.xy;
    vec2 p1    = gl_in[2].gl_Position.xy;
    vec2 pNext = gl_in[3].gl_Position.xy;

    // Aspect-corrected vectors for accurate angles
    vec2 v0 = normalize(aspectCorrected(p0 - pPrev));
    vec2 v1 = normalize(aspectCorrected(p1 - p0));
    vec2 v2 = normalize(aspectCorrected(pNext - p1));

    // Normals (also aspect-corrected)
    vec2 n0 = vec2(-v0.y, v0.x);
    vec2 n1 = vec2(-v1.y, v1.x);
    vec2 n2 = vec2(-v2.y, v2.x);

    // GL_LINE_STRIP_ADJACENCY does not provide adjacency information for
    // the first and last segments, since there is no preceding (pPrev) or
    // succeeding (pLast) vertex, respectively. Therefore, the first and last
    // segments must be drawn separately in addition to the segments from the
    // second to the second-to-last vertex.
    if (vIndex[0] == 0)
    {
        emitQuad(n0, n0, n1, pPrev, p0);
    }
    else if (vIndex[3] == numVertices - 1)
    {
        emitQuad(n0, n1, n1, p1, pNext);
    }

    emitQuad(n0, n1, n2, p0, p1);
}
