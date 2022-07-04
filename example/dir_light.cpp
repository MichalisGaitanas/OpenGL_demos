#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<cstdio>
#include<cmath>
#include"../include/shader.hpp"
#include"../include/mesh.hpp"

int win_width = 900, win_height = 900;
const char *win_label = "Directional light calculations";

void raw_hardware_input(GLFWwindow *win)
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
    GLFWwindow *window = glfwCreateWindow(win_width, win_height, win_label, NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    //force the window to open at the center of the screen
    int monitx, monity;
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorPos(monitor, &monitx, &monity);
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwSetWindowPos( window, (monitx + mode->width - win_width)/2,
                              (monity + mode->height - win_height)/2 );
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //validate glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...");
        return 0;
    }

    ////////////////////////////////////////////////////////////////////////////

    mesh sphere("../obj/vert_face_snorm/sphere_rad1.obj", 1,1,1);
    shader shad("../shader/vertex/trans_mvpn.vert", "../shader/fragment/dir_light_ads.frag");
    shad.use();

    glm::vec3 light_dir; //light direction in world coordinates
    glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f); //lighting calculations color
    glm::vec3 sphere_col = glm::vec3(0.85f,0.4f,0.0f); //model color
    glm::vec3 cam_pos = glm::vec3(0.0f,-5.0f,2.0f); //camera position in world coordinates
    glm::vec3 cam_aim = glm::vec3(0.0f,0.0f,0.0f); //camera aim (eye) direction
    glm::vec3 cam_up = glm::vec3(0.0f,0.0f,1.0f); //camera up direction

    //projection, view and model matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),(float)win_width/win_height, 0.01f,100.0f);
    glm::mat4 view = glm::lookAt(cam_pos, cam_aim, cam_up);
    glm::mat4 model = glm::mat4(1.0f);

    shad.set_mat4_uniform("projection", projection);
    shad.set_mat4_uniform("view", view);
    shad.set_mat4_uniform("model", model);
    shad.set_vec3_uniform("cam_pos", cam_pos);
    shad.set_vec3_uniform("light_col", light_col);
    shad.set_vec3_uniform("model_col", sphere_col);

    ////////////////////////////////////////////////////////////////////////////

    glEnable(GL_DEPTH_TEST); //enable depth - testing
    glClearColor(0.0f,0.0f,0.0f,1.0f); //background color
    while (!glfwWindowShouldClose(window)) //game loop
    {
        raw_hardware_input(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //color buffer and z-buffer

        light_dir = glm::vec3(cos(glfwGetTime()),sin(glfwGetTime()),0.0f); //light direction in world coordinates
        shad.set_vec3_uniform("light_dir", light_dir);
        sphere.draw_triangles();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
