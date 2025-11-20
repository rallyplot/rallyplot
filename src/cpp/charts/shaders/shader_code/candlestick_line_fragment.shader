#version 330 core
/*
    The fragment shader takes the color and fragment position
    from the candlestick vertex shader. If lampMode is 1,
    diffiuse and specular light will be cast over the candlesticks.
    This only really works with a dark background.
*/

in vec4 gColor;
in vec4 gFragPos;
out vec4 FragColor;


void main()
{
    FragColor = gColor;
}
