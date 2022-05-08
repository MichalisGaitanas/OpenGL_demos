#version 330 core

in vec3 fragPos;
in vec3 normal;

out vec4 finalCol; //color of the fragment after lighting calculations

uniform vec3 lightDir; //direction of the light in world coordinates
uniform vec3 lightCol; //light color
uniform vec3 modelCol; //model color

void main()
{
    //ambient color component
    float ambient = 0.15f;

    //diffuse color component
    vec3 norm = normalize(normal);
    vec3 lightDirNorm = normalize(lightDir); //light direction with respect to model coordinates
    float diffuse = max(dot(norm,lightDirNorm), 0.0f);

    vec3 fragCol = (ambient + diffuse)*modelCol*lightCol;
    finalCol = vec4(fragCol, 1.0f);
}
