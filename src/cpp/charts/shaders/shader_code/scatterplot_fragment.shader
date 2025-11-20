#version 330 core
in vec2 texCoords;

out vec4 FragColor;

uniform vec4 color;
uniform sampler2D shapeTexture;

void main() {

    vec4 texColor = texture(shapeTexture, texCoords);
    if (texColor.a < 0.99999) discard; // Transparency check for circular shape
    FragColor = texColor * color;

}
