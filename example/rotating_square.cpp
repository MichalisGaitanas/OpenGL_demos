#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<iostream>
#include"../headers/shader.h"

using namespace std;

int width = 800, height = 800;
const char * label = "Rotating square";

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

    GLFWwindow *win = glfwCreateWindow(width, height, label, NULL, NULL);
    if (win == NULL)
    {
        cout << "Failed to create glfw window. Exiting...\n";
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(win);
    int monitx, monity;
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorPos(monitor, &monitx, &monity);
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwSetWindowPos( win, (monitx + mode->width - width)/2, (monity + mode->height - height)/2 );
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);

    //validate glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize glew. Exiting...\n";
        return 0;
    }

    //define the coordinates of the vertices of the square to be rendered
    float verts[] = { -0.5f, -0.5f, 0.0f,
                       0.5f, -0.5f, 0.0f,
                       0.5f,  0.5f, 0.0f, //first triangle

                      -0.5f, -0.5f, 0.0f,
                       0.5f,  0.5f, 0.0f,
                      -0.5f,  0.5f, 0.0f }; //second triangle

    unsigned int vao,vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0 );
    glEnableVertexAttribArray(0);

    shader shad("../shaders/trans_m.vert","../shaders/monochromatic.frag"); //shader objects
    glm::vec3 modelCol = glm::vec3(1.0f,0.1f,0.1f);

    glClearColor(1.0f,1.0f,1.0f,1.0f); //background color
    while (!glfwWindowShouldClose(win)) //rendering loop
    {
        process_hardware_inputs(win);

        glm::mat4 model = glm::mat4(1.0f); //all vertices have identity matrix
        model = glm::rotate(model, sin((float)glfwGetTime()), glm::vec3(0.0f,0.0f,1.0f)); //rotate all vertices around the z-axis

        glClear(GL_COLOR_BUFFER_BIT);

        shad.use(); //activate the "shad" shader object
        shad.set_mat4_uniform("model", model); //pass the model variable to the shaders
        shad.set_vec3_uniform("modelCol",modelCol);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);

    glfwTerminate();
    return 0;
}
