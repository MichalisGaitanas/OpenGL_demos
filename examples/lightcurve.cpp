#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"
#include"../imgui/implot.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include<cstdio>
#include<cmath>

#include"../include/shader.hpp"
#include"../include/mesh.hpp"

int win_width, win_height;
float aspect_ratio;

void raw_hardware_input(GLFWwindow *win)
{
    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(win, true);
}

void framebuffer_size_callback(GLFWwindow *win, int w, int h)
{
    glViewport(0,0,w,h);
    win_width = w;
    win_height = h;
    aspect_ratio = (float)w/h;
}

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    
    GLFWwindow *win = glfwCreateWindow(800, 600, "65803 Didymos", NULL, NULL);
    if (win == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    
    glfwMakeContextCurrent(win);
    
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        glfwTerminate();
        return 0;
    }

    glfwSetWindowSizeLimits(win, 400, 400, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
    
    glfwGetWindowSize(win, &win_width, &win_height);
    aspect_ratio = (float)win_width/win_height;

    meshvfn body("../obj/vfn/gerasimenko256k.obj"); //flat normals
    //meshvfn body("../obj/vfn/gerasimenko256k_smooth.obj"); //smooth normals
    //meshvfn body("../obj/vfn/didymos/didymain2019.obj"); //flat normals
    //meshvfn body("../obj/vfn/didymos/didymain2019_smooth.obj"); //smooth normals
    //meshvfn body("../obj/vfn/uv_sphere_rad1_40x30.obj"); //smooth normals

    shader shad("../shaders/vertex/trans_mvpn.vert","../shaders/fragment/lightcurve.frag");
    shad.use();

    glm::vec3 light_dir; //light direction in world coordinates
    glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f); //lighting calculation color
    glm::vec3 body_col = glm::vec3(1.0f,1.0f,1.0f); //model color
    
    glm::vec3 cam_pos = glm::vec3(0.0f,-5.0f,2.0f); //camera position in world coordinates
    glm::vec3 cam_aim = glm::vec3(0.0f,0.0f,0.0f); //camera aim direction
    glm::vec3 cam_up = glm::vec3(0.0f,0.0f,1.0f); //camera up direction

    //projection, view and model matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)win_width/win_height, 0.01f,1000.0f);
    glm::mat4 view = glm::lookAt(cam_pos, cam_aim, cam_up);
    glm::mat4 model = glm::mat4(1.0f);

    shad.set_mat4_uniform("projection", projection);
    shad.set_mat4_uniform("view", view);
    shad.set_mat4_uniform("model", model);

    shad.set_vec3_uniform("cam_pos", cam_pos);
    shad.set_vec3_uniform("light_col", light_col);
    shad.set_vec3_uniform("body_col", body_col);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.05f,0.05f,0.05f,1.0f);
    while (!glfwWindowShouldClose(win))
    {
        raw_hardware_input(win);

        //refresh the color and z buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        light_dir = glm::vec3(cos(glfwGetTime()),sin(glfwGetTime()),0.0f); //light direction in world coordinates
        shad.set_vec3_uniform("light_dir", light_dir);
        body.draw_triangles();

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
