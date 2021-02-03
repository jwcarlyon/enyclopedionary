#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewerPos;
uniform sampler2D texture_diffuse1;

void main()
{
    vec4 modelColor = texture(texture_diffuse1, TexCoords);

    float ambientStrength = 0.1;
    float specularStrength = 0.5;
    vec3 ambient = ambientStrength * lightColor;
    vec3 norm = normalize(Normal);
    vec3 lightDirection = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewerPos - FragPos);

    float diffuse_magnitude = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = (lightColor * diffuse_magnitude);
    vec3 reflectDir = reflect(-lightDirection, norm);
    float specular_magnitude = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * specular_magnitude * lightColor;

    vec3 result = modelColor.xyz * (ambient + diffuse + specular);
    FragColor = vec4(result, 1.0);
}
