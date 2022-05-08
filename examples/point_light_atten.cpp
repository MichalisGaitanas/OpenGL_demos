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
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE); //full screen (windowed)

    GLFWwindow *window = glfwCreateWindow(500, 500, "Point light with attenuation", NULL, NULL);
    if (window == NULL)
    {
        cout << "Failed to create glfw window. Exiting...\n";
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); //set framebuffer size callback function

    //calculate window dimensions
    int winWidth, winHeight;
    glfwGetWindowSize(window, &winWidth, &winHeight);

    //validate glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize glew. Exiting...\n";
        return 0;
    }

    //create meshes
    mesh suzanne("../models/smooth/suzanne.obj", 1,1,1);
    mesh lamp("../models/smooth/sphere.obj", 1,1,0);

    //create shaders
    shader suzanneShad("../shaders/trans_mvpn.vert","../shaders/point_light_atten_ads.frag");
    shader lampShad("../shaders/trans_mvp.vert","../shaders/monochromatic.frag");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.05f,0.05f,0.05f,1.0f);
    while (!glfwWindowShouldClose(window))
    {
        process_hardware_inputs(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 lightPos = glm::vec3(0.0f, -5.0f + 3.0f*sin(glfwGetTime()), 0.0f); //light position in world coordinates
        glm::vec3 lightCol = glm::vec3(1.0f,1.0f,1.0f); //lighting calculations color
        glm::vec3 modelCol = glm::vec3(0.8f,0.4f,0.0f); //suzanne color
        glm::vec3 lampCol = glm::vec3(1.0f,1.0f,1.0f); //lamp color
        glm::vec3 camPos = glm::vec3(0.0f,-15.0f,7.5f); //camera position in world coordinates

        glm::mat4 projection = glm::perspective(glm::radians(45.0f),(float)winWidth/(float)winHeight, 0.01f,100.0f);
        glm::mat4 view = glm::lookAt(camPos, glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,1.0f));
        glm::mat4 model = glm::mat4(1.0f);

        suzanneShad.use();
        suzanneShad.set_mat4_uniform("projection", projection);
        suzanneShad.set_mat4_uniform("view", view);
        suzanneShad.set_mat4_uniform("model", model);
        suzanneShad.set_vec3_uniform("lightPos", lightPos);
        suzanneShad.set_vec3_uniform("camPos", camPos);
        suzanneShad.set_vec3_uniform("lightCol", lightCol);
        suzanneShad.set_vec3_uniform("modelCol", modelCol);
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
