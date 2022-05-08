#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<iostream>
#include"../headers/shader.h"
#include"../headers/mesh.h"

using namespace std;

int winWidth = 900, winHeight = 900;
const char *label = "Model loading";

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

    //create a window object at the center of the screen
    GLFWwindow *window = glfwCreateWindow(winWidth, winHeight, label, NULL, NULL);
    if (window == NULL)
    {
        cout << "Failed to create glfw window. Exiting...\n";
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    int monitx, monity; //monitor's top left corner coordinates in the virtual space
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorPos(monitor, &monitx, &monity);
    cout << "Monitor's coordinates in virtual space : (" << monitx << "," << monity << ")\n";
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwSetWindowPos( window, (monitx + mode->width - winWidth)/2, (monity + mode->height - winHeight)/2 );
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); //set framebuffer size callback function

    //validate glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize glew. Exiting...\n";
        return 0;
    }

    mesh suzanne("../models/smooth/suzanne.obj", 1,1,1);
    mesh lamp("../models/flat/cube.obj", 1,1,0);

    //create shaders
    shader suzanneShad("../shaders/trans_mvpn.vert","../shaders/point_light_ads.frag");
    shader lampShad("../shaders/trans_mvp.vert","../shaders/monochromatic.frag");

    glm::vec3 lightPos = glm::vec3(0.0f,-2.5f,1.0f); //light position in world coordinates
    glm::vec3 lightCol = glm::vec3(1.0f,1.0f,1.0f); //lighting calculations color
    glm::vec3 suzanneCol = glm::vec3(0.5f,0.5f,0.5f); //suzanne color
    glm::vec3 lampCol = glm::vec3(1.0f,1.0f,1.0f); //lamp color
    glm::vec3 camPos = glm::vec3(-5.0f,-5.0f,1.0f); //camera position in world coordinates
    glm::vec3 camAim = glm::vec3(0.0f,0.0f,0.0f); //camera aim direction
    glm::vec3 camUp = glm::vec3(0.0f,0.0f,1.0f); //camera up direction

    glm::mat4 projection = glm::perspective(glm::radians(45.0f),(float)winWidth/(float)winHeight, 0.01f,100.0f);
    glm::mat4 view = glm::lookAt(camPos, camAim, camUp);
    glm::mat4 model = glm::mat4(1.0f);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    while (!glfwWindowShouldClose(window)) //render loop
    {
        process_hardware_inputs(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        model = glm::mat4(1.0f);
        model = glm::rotate(model, (0.5f*(float)glfwGetTime()), glm::vec3(1.0f,1.0f,1.0f));
        suzanneShad.use();
        suzanneShad.set_mat4_uniform("projection", projection);
        suzanneShad.set_mat4_uniform("view", view);
        suzanneShad.set_mat4_uniform("model", model);
        suzanneShad.set_vec3_uniform("lightPos", lightPos);
        suzanneShad.set_vec3_uniform("camPos", camPos);
        suzanneShad.set_vec3_uniform("lightCol", lightCol);
        suzanneShad.set_vec3_uniform("modelCol", suzanneCol);
        suzanne.draw_triangles();

        lampShad.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.25f,0.25f,0.25f));
        lampShad.set_mat4_uniform("projection", projection);
        lampShad.set_mat4_uniform("view", view);
        lampShad.set_mat4_uniform("model", model);
        lampShad.set_vec3_uniform("modelCol", lampCol);
        lamp.draw_triangles();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
