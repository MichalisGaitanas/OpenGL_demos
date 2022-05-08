#version 330 core

in vec3 fragPos;
in vec3 normal;

out vec4 finalCol; //color of the fragment after lighting calculations

uniform vec3 lightPos; //position of the light in world coordinates
uniform vec3 lightCol; //light color
uniform vec3 modelCol; //model color
uniform vec3 camPos; //position of the camera in world coordinates

void main()
{    
    //ambient color component
    float ambient = 0.15f;
    
    //diffuse color component
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos); //light direction with respect to model coordinates
    float diffuse = max(dot(norm,lightDir), 0.0f);
    
    //specular color component
    vec3 viewDir = normalize(camPos - fragPos); //camera direction with respect to model coordinates
    vec3 reflectDir = reflect(-lightDir,norm); //ray's reflection direction with respect to model coordinates
    float specular = 0.5f*pow(max(dot(viewDir,reflectDir),0.0f), 128);
    
    vec3 fragCol = (ambient + diffuse + specular)*modelCol*lightCol;
    finalCol = vec4(fragCol, 1.0f);
}





