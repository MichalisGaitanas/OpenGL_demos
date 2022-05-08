#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<iostream>
#include<cmath>
#include"../headers/shader.h"
#include"../headers/mesh.h"

using namespace std;

int winWidth = 900, winHeight = 900;
const char *winLabel = "Bloom effect";

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
    //initialize glfw and some hints
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //create the window object
    GLFWwindow *window = glfwCreateWindow(winWidth, winHeight, winLabel, NULL, NULL);
    if (window == NULL)
    {
        cout << "Failed to create glfw window. Exiting...\n";
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    //force the window to open at the center of the screen
    int monitx, monity;
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorPos(monitor, &monitx, &monity);
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwSetWindowPos( window, (monitx + mode->width - winWidth)/2,
                              (monity + mode->height - winHeight)/2 );
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //validate glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize glew. Exiting..." << endl;
        return 0;
    }

    ////////////////////////////////////////////////////////////////////////////

    mesh sphere("../models/smooth/sphere.obj", 1,1,0);
    shader shad("../shaders/bloom.vert","../shaders/bloom.frag");
    shad.use();

    glm::vec3 lightCol = glm::vec3(1.0f,1.0f,1.0f); //lighting calculations color
    glm::vec3 sphereCol = glm::vec3(0.85f,0.4f,0.0f); //model color

    glm::vec3 camPos = glm::vec3(0.0f,-5.0f,0.0f); //camera position in world coordinates
    glm::vec3 camAim = glm::vec3(0.0f,0.0f,0.0f); //camera aim (eye) direction
    glm::vec3 camUp = glm::vec3(0.0f,0.0f,1.0f); //camera up direction

    //projection, view and model matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),(float)winWidth/winHeight, 0.01f,100.0f);
    glm::mat4 view = glm::lookAt(camPos, camAim, camUp);
    glm::mat4 model;

    shad.set_mat4_uniform("projection", projection);
    shad.set_mat4_uniform("view", view);
    shad.set_vec3_uniform("lightCol", lightCol);
    shad.set_vec3_uniform("modelCol", sphereCol);

    glEnable(GL_DEPTH_TEST); //enable depth - testing
    glClearColor(0.0f,0.0f,0.0f,1.0f); //background color
    while (!glfwWindowShouldClose(window)) //game loop
    {
        double tm = glfwGetTime();
        process_hardware_inputs(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //color buffer and z-buffer

        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(1.0f,0.1f,1.0f));
        model = glm::translate(model, glm::vec3(0.0f,0.0f,sin(tm)));
        model = glm::rotate(model, (float)tm, glm::vec3(0.0f,0.0f,1.0f));
        shad.set_mat4_uniform("model", model);
        sphere.draw_triangles();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
