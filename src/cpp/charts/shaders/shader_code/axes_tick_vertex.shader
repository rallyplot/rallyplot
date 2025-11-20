/*
	This vertex shader is used to setup both the axis ticks and gridlines,
	controlled by isGridline argument.

	Ticks are setup based on 30 vertices, and the start tick position
	and tick delta specified in passed uniform variable. The number of
	vertices plot (i.e. tick numbers) is set by the draw call.

	The vertices are organised as arrays where the first element
	is the index (e.g. 5th tick) and the second is whether it is 
	the tick start or tick end. The position sets the position on 
	the axis (for example if isX axis, position is x coordinate)
	while the extension sets the width/height (e.g. if isX, the 
	extension are the y coordinates).

	If we are plotting the gridline, we extend from the axis
	to all the way across the plot. Otherwise, we extend only
	to/from the tick width/height.
*/
#version 330 core

layout (location = 0) in vec2 Index;

uniform float tickHeight;
uniform float startPos;
uniform float tickDelta;
uniform float axisPos;

uniform int startAtLowestIdx;
uniform int isX;
uniform int isGridline;


uniform mat4 axes_viewport_transform;

void main()
{
    // Set the tick position on the axis
    // ---------------------------------------------------------------------------

    float tickPosition;

    if (startAtLowestIdx == 1)
        tickPosition = startPos + Index.x * tickDelta;
    else
        tickPosition = startPos - Index.x * tickDelta;

    // Set the width / height (for ticks vs. gridlines)
    // ---------------------------------------------------------------------------

    float tickExtension;

    if (isGridline == 0)
    {
        if (Index.y > 0.0f)
            tickExtension = axisPos - tickHeight / 2.0f;
        else
            tickExtension = axisPos; //  + tickHeight / 2.0f;  only show half-tick on x-axis
    }
    else
    {
        if (Index.y > 0.0f)
            tickExtension = -1.0f;
        else
            tickExtension = 1.0f;
    }

    // Set the positions based on whether we are on the X or Y axis.
    // ---------------------------------------------------------------------------

    float xPos;
    float yPos;

    if (isX == 1)
    {
        xPos = tickPosition;
        yPos = tickExtension;
    }
    else
    {
        xPos = tickExtension;
        yPos = tickPosition;
    }
    gl_Position = axes_viewport_transform  * vec4(xPos, yPos, 0.0f, 1.0f);

}
