#version 330 core

in vec3 fragPos;
in vec3 normal;

out vec4 finalCol; //color of the fragment after lighting calculations

uniform vec3 lightPos; //position of the light in world coordinates
uniform vec3 lightCol; //light color
uniform vec3 modelCol; //model color
uniform vec3 camFront; //view front vector
uniform float cosCuttoff; //cosine of the cone angle (separates the flashlight calculations with the rest of the world's lighting calculations)

void main()
{
    //ambient color component
    float ambient = 0.15f;
    
    vec3 lightDir = normalize(lightPos - fragPos);
    float cosFrag = dot(-lightDir, normalize(camFront));
    float diffuse; //diffuse color component
    if (cosFrag >= cosCuttoff)
    {
        vec3 norm = normalize(normal);
        diffuse = max(dot(norm,lightDir), 0.0f);
    }
    else
    {
        diffuse = 0.0f;
    }
    
    //final fragment color
    vec3 fragCol = (ambient + diffuse)*modelCol*lightCol;
    finalCol = vec4(fragCol, 1.0f);
}





