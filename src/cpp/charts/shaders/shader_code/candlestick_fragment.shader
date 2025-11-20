#version 330 core
/*
        The fragment shader takes the color and fragment position
        from the candlestick vertex shader. If lampMode is 1,
        diffiuse and specular light will be cast over the candlesticks.
        This only really works with a dark background.
*/
in vec4 Color;
in vec4 FragPos;
out vec4 FragColor;

uniform int lampMode;

void main()
{
    if (lampMode == 1)
    {
        // Setup the light color, normal as poitning out of
        // the plot, and light position (top right at present).

        vec3 lightColor = vec3(5, 5, 5);  // TODO: expose, change light brightness
        vec3 normal = vec3(0.0, 0.0, 1.0);
        vec4 lightPos = vec4(0.0, 0.2, 0.8, 1.0);  // TODO: expose, change to move light

        // Compute the light path and reflection
        vec3 lightPath = normalize(vec3(lightPos - FragPos));

        vec3 reflectDir = reflect(-lightPath,  normal);  // -?
        vec4 viewPos = vec4(0.5, 0.5, 1.0, 1.0);

        vec3 viewDirection = normalize(vec3(viewPos - FragPos));

        // Compute the diffuse and specular scalars
        float spec = pow(max(dot(viewDirection, reflectDir), 0.0), 128.0);  // TODO: expose, change for decay

        float diffDot = max(dot(lightPath, normal), 0.0);

        // Apply the light transform to the candlestick Color
        vec3 diffuse =  vec3(Color.x, Color.y, Color.z) * lightColor * pow(diffDot, 4.0) * (1 / (1 + 2 * length(vec3(lightPos - FragPos))));
        vec3 specular = vec3(Color.x, Color.y, Color.z) * spec * vec3(2.0, 2.0, 2.0);

        vec3 FragColor_ = diffuse;  // specular
        FragColor = vec4(FragColor_, 1.0);
    }
    else
    {
        FragColor = Color;
    }
}
