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
    
    //attenuation factor
    float lightDist = length(lightPos - fragPos); //distance between point light source and fragment
    float k1 = 1.0f, k2 = 0.09f, k3 = 0.032f; //constant (k1), linear (k2) and quadratic (k3) attenuation parameters
    float atten = 1.0f/( k1 + k2*lightDist + k3*lightDist*lightDist );
    
    vec3 fragCol = (ambient + diffuse + specular)*modelCol*lightCol*atten;
    finalCol = vec4(fragCol, 1.0f);
}
