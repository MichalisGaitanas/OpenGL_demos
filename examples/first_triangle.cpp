#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<iostream>

using namespace std;

const int xTopLeft = 100, yTopLeft = 100; //top left window coordinates
const int winWidth = 800, winHeight = 700;
const char *winLabel = "First triangle";

//vertex shader source code
const char *vSource = "#version 330 core\n"
                      "layout (location = 0) in vec3 pos;\n"
                      "void main()\n"
                      "{\n"
                      "   gl_Position = vec4(pos.x, pos.y, pos.z, 1.0f);\n"
                      "}";

//fragment shader source code
const char *fSource = "#version 330 core\n"
                      "out vec4 fragColor;\n"
                      "void main()\n"
                      "{\n"
                      "   fragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
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
    glfwSetWindowPos(win, xTopLeft,yTopLeft); //set initial position of the window
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

    //tell the computer how to store and read the previously defined geometry - data in order to be able to process/render it
    unsigned int vao,vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0 );
    glEnableVertexAttribArray(0);

    //read and compile vertex shader
    unsigned int vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vSource, NULL);
    glCompileShader(vShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vShader, GL_COMPILE_STATUS, &success);
    if (!success) //report compilation error
    {
        glGetShaderInfoLog(vShader, 512, NULL, infoLog);
        cout << "Error while compiling VERTEX shader.\n";
        cout << infoLog << "\n";
    }

    //read and compile fragment shader
    unsigned int fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fSource, NULL);
    glCompileShader(fShader);
    glGetShaderiv(fShader, GL_COMPILE_STATUS, &success);
    if (!success) //report compilation error
    {
        glGetShaderInfoLog(fShader, 512, NULL, infoLog);
        cout << "Error while compiling FRAGMENT shader.\n";
        cout << infoLog << "\n";
    }

    //create and link a "chained" shader program(s)
    unsigned int shaderProg = glCreateProgram();
    glAttachShader(shaderProg, vShader);
    glAttachShader(shaderProg, fShader);
    glLinkProgram(shaderProg);
    glGetProgramiv(shaderProg, GL_LINK_STATUS, &success);
    if (!success) //report linkage error
    {
        glGetProgramInfoLog(shaderProg, 512, NULL, infoLog);
        cout << "Error while linking shader program.\n";
        cout << infoLog << "\n";
    }

    glClearColor(0.1f,0.5f,0.2f,1.0f); //background color
    while (!glfwWindowShouldClose(win)) //game loop
    {
        process_hardware_inputs(win);

        glClear(GL_COLOR_BUFFER_BIT); //constantly refresh the backgound color in accordance with the glClearColor()

        glUseProgram(shaderProg);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3); //actually plot

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
