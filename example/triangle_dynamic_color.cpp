#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<iostream>
#include<cmath>

using namespace std;

const int winWidth = 900, winHeight = 750;
const char *winLabel = "Triangle dynamic intensity";

//vertex shader source code
const char *vSource = "#version 330 core\n"
                      "layout (location = 0) in vec3 pos;\n"
                      "void main()\n"
                      "{\n"
                      "   gl_Position = vec4(pos, 1.0f);\n"
                      "}";

//fragment shader source code
const char *fSource = "#version 330 core\n"
                      "out vec4 fragColor;\n"
                      "uniform float intensity;\n"
                      "void main()\n"
                      "{\n"
                      "   fragColor = intensity*vec4(1.0f,0.0f,0.0f,1.0f);\n"
                      "}";

void process_hardware_inputs(GLFWwindow *win)
{
    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(win, true);
    }
}

void framebuffer_size_callback(GLFWwindow *win, int w, int h)
{
    glViewport(0,0,w,h);
}

int main()
{
    //initialize glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    //create a window object
    GLFWwindow *win = glfwCreateWindow(winWidth, winHeight, winLabel, NULL, NULL);
    if (win == NULL)
    {
        cout << "Failed to create glfw window. Exiting...\n";
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(win);
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
    
    //validate glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize glew. Exiting...\n";
        return 0;
    }
    
    //define the coordinates of the vertices of the triangle to be rendered
    float verts[] = { -0.5f, -0.5f, 0.0f,
                       0.5f, -0.5f, 0.0f,
                       0.0f,  0.5f, 0.0f };
    
    unsigned int vao,vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0 );
    glEnableVertexAttribArray(0);
    
    //vertex shader stuff
    unsigned int vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vSource, NULL);
    glCompileShader(vShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vShader, 512, NULL, infoLog);
        cout << "Error while compiling vertex shader.\n";
        cout << infoLog << "\n";
    }
    
    //fragment shader stuff
    unsigned int fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fSource, NULL);
    glCompileShader(fShader);
    glGetShaderiv(fShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fShader, 512, NULL, infoLog);
        cout << "Error while compiling fragment shader.\n";
        cout << infoLog << "\n";
    }
    
    //linking stuff
    unsigned int shaderProg = glCreateProgram();
    glAttachShader(shaderProg, vShader);
    glAttachShader(shaderProg, fShader);
    glLinkProgram(shaderProg);
    glGetProgramiv(shaderProg, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProg, 512, NULL, infoLog);
        cout << "Error while linking shader program.\n";
        cout << infoLog << "\n";
    }
    
    glClearColor(0.1f,0.1f,0.1f,1.0f); //background color
    while (!glfwWindowShouldClose(win)) //game loop
    {
        process_hardware_inputs(win);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProg);
            float t = glfwGetTime(); //elapsed time in seconds since glfwInit() was called
            cout << "\r" << int(t) << " sec"; //print time in the terminal
            cout.flush();
            float intensity = abs(cos(t)); //let the intensity of the triangle be dynamic (changing) based on f(t) = |sin(t)|
            int intensityLocation = glGetUniformLocation(shaderProg, "intensity"); //intensity variable location so that the shaders know about it
            glUniform1f(intensityLocation, intensity);
            glBindVertexArray(vao);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(win);
        glfwPollEvents();
    }
    
    //free resources
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shaderProg);
    glDeleteShader(vShader);
    glDeleteShader(fShader);
    
    glfwTerminate();
    return 0;
}
