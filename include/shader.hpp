#ifndef SHADER_HPP
#define SHADER_HPP

#include<GL/glew.h>
#include<glm/glm.hpp>
#include<iostream>
#include<fstream>
#include<string>

using namespace std;

class shader
{
public:
    unsigned ID; //shader program ID
    shader(const char *vPath, const char *fPath) //constructor
    {
        //read the vertex shader source code from its file
        ifstream fpVertex(vPath);
        if (!fpVertex.is_open())
        {
            cout << "Vertex shader source file not found. Exiting...\n";
            exit(EXIT_FAILURE);
        }
        string vTemp;
        vTemp.assign( (istreambuf_iterator<char>(fpVertex)), (istreambuf_iterator<char>()) );
        const char *vSource = vTemp.c_str();
        
        //compile the shader and check for errors
        unsigned vShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vShader, 1, &vSource, NULL);
        glCompileShader(vShader);
        int success;
        char infoLog[1024];
        glGetShaderiv(vShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vShader, 1024, NULL, infoLog);
            cout << "Error while compiling vertex shader.\n";
            cout << infoLog << "\n";
        }
        
        ////////////////////////////////////////////////////////////////////////
        
        //read the fragment shader source code from its file
        ifstream fpFragment(fPath);
        if (!fpFragment.is_open())
        {
            cout << "Fragment shader source file not found. Exiting...\n";
            exit(EXIT_FAILURE);
        }
        string fTemp;
        fTemp.assign( (istreambuf_iterator<char>(fpFragment)), (istreambuf_iterator<char>()) );
        const char *fSource = fTemp.c_str();
        
        //compile the shader and check for errors
        unsigned fShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fShader, 1, &fSource, NULL);
        glCompileShader(fShader);
        glGetShaderiv(fShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fShader, 1024, NULL, infoLog);
            cout << "Error while compiling fragment shader.\n";
            cout << infoLog << "\n";
        }
        
        ////////////////////////////////////////////////////////////////////////
        
        //handle linking
        ID = glCreateProgram();
        glAttachShader(ID, vShader);
        glAttachShader(ID, fShader);
        glLinkProgram(ID);
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(ID, 1024, NULL, infoLog);
            cout << "Error while linking shader program.\n";
            cout << infoLog << "\n";
        }
        
        glDeleteShader(vShader);
        glDeleteShader(fShader);
    }
    
    //shader activation function
    void use()
    {
        glUseProgram(ID);
    }
    
    //communicate with the shaders through a simple int uniform
    void set_int_uniform(const string &name, int value)
    {
        unsigned location = glGetUniformLocation(ID, name.c_str());
        glUniform1i(location, value);
    }
    
    //communicate with the shaders through a simple float uniform
    void set_float_uniform(const string &name, float value)
    {
        unsigned location = glGetUniformLocation(ID, name.c_str());
        glUniform1f(location, value);
    }
    
    //communicate with the shaders through 2 float uniforms
    void set_vec2_uniform(const string &name, float x, float y)
    {
        unsigned location = glGetUniformLocation(ID, name.c_str());
        glUniform2f(location, x,y);
    }
    
    //communicate with the shaders through a vector of 2 float uniforms
    void set_vec2_uniform(const string &name, glm::vec2 &v)
    {
        unsigned location = glGetUniformLocation(ID, name.c_str());
        glUniform2fv(location, 1, &v[0]);
    }
    
    //communicate with the shaders through 3 float uniforms
    void set_vec3_uniform(const string &name, float x, float y, float z)
    {
        unsigned location = glGetUniformLocation(ID, name.c_str());
        glUniform3f(location, x,y,z);
    }
    
    //communicate with the shaders through a vector of 3 float uniforms
    void set_vec3_uniform(const string &name, glm::vec3 &v)
    {
        unsigned location = glGetUniformLocation(ID, name.c_str());
        glUniform3fv(location, 1, &v[0]);
    }
    
    //communicate with the shaders through 4 float uniforms
    void set_vec4_uniform(const string &name, float x, float y, float z, float w)
    {
        unsigned location = glGetUniformLocation(ID, name.c_str());
        glUniform4f(location, x,y,z,w);
    }
    
    //communicate with the shaders through a vector of 4 float uniforms
    void set_vec4_uniform(const string &name, glm::vec4 &v)
    {
        unsigned location = glGetUniformLocation(ID, name.c_str());
        glUniform4fv(location, 1, &v[0]);
    }
    
    //communicate with the shaders through a 2x2 matrix of floats
    void set_mat2_uniform(const string &name, glm::mat2 &m)
    {
        unsigned location = glGetUniformLocation(ID, name.c_str());
        glUniformMatrix2fv(location, 1, GL_FALSE, &m[0][0]);
    }
    
    //communicate with the shaders through a 3x3 matrix of floats
    void set_mat3_uniform(const string &name, glm::mat3 &m)
    {
        unsigned location = glGetUniformLocation(ID, name.c_str());
        glUniformMatrix3fv(location, 1, GL_FALSE, &m[0][0]);
    }
    
    //communicate with the shaders through a 4x4 matrix of floats
    void set_mat4_uniform(const string &name, glm::mat4 &m)
    {
        unsigned location = glGetUniformLocation(ID, name.c_str());
        glUniformMatrix4fv(location, 1, GL_FALSE, &m[0][0]);
    }
};

#endif