#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<cstdio>
#include<cmath>
#include"../include/shader.hpp"

int width = 800, height = 800;
const char *label = "Rotating cube";

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
    //initialize glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(width, height, label, NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    int monitx, monity;
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorPos(monitor, &monitx, &monity);
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwSetWindowPos( window, (monitx + mode->width - width)/2, (monity + mode->height - height)/2 );
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //validate glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }

    //define the coordinates of the vertices of the cube to be rendered
    float verts[] = { -0.5f, -0.5f, -0.5f,
                       0.5f, -0.5f, -0.5f,
                       0.5f,  0.5f, -0.5f,
                       0.5f,  0.5f, -0.5f,
                      -0.5f,  0.5f, -0.5f,
                      -0.5f, -0.5f, -0.5f, //down face

                      -0.5f, -0.5f,  0.5f,
                       0.5f, -0.5f,  0.5f,
                       0.5f,  0.5f,  0.5f,
                       0.5f,  0.5f,  0.5f,
                      -0.5f,  0.5f,  0.5f,
                      -0.5f, -0.5f,  0.5f, //up face

                      -0.5f,  0.5f,  0.5f,
                      -0.5f,  0.5f, -0.5f,
                      -0.5f, -0.5f, -0.5f,
                      -0.5f, -0.5f, -0.5f,
                      -0.5f, -0.5f,  0.5f,
                      -0.5f,  0.5f,  0.5f, //left face

                       0.5f,  0.5f,  0.5f,
                       0.5f,  0.5f, -0.5f,
                       0.5f, -0.5f, -0.5f,
                       0.5f, -0.5f, -0.5f,
                       0.5f, -0.5f,  0.5f,
                       0.5f,  0.5f,  0.5f, //right face

                      -0.5f, -0.5f, -0.5f,
                       0.5f, -0.5f, -0.5f,
                       0.5f, -0.5f,  0.5f,
                       0.5f, -0.5f,  0.5f,
                      -0.5f, -0.5f,  0.5f,
                      -0.5f, -0.5f, -0.5f, //back face

                      -0.5f,  0.5f, -0.5f,
                       0.5f,  0.5f, -0.5f,
                       0.5f,  0.5f,  0.5f,
                       0.5f,  0.5f,  0.5f,
                      -0.5f,  0.5f,  0.5f,
                      -0.5f,  0.5f, -0.5f  }; //front face

    unsigned int vao,vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0 );
    glEnableVertexAttribArray(0);

    shader shad("../shader/vertex/trans_mp.vert","../shader/fragment/monochromatic.frag");
    shad.use();

    glEnable(GL_DEPTH_TEST); //enable depth - testing to correctly render 3D stuff
    glClearColor(0.0f,0.0f,0.0f,1.0f); //background color
    while (!glfwWindowShouldClose(window)) //game loop
    {
        raw_hardware_input(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 model_col = glm::vec3(1.0f,0.1f,0.1f);
        shad.set_vec3_uniform("model_col",model_col);

        //width, height are never updated because there's no corresponding "update code" in framebuffer_size_callback()
        glm::mat4 projection = glm::perspective(glm::radians(45.0f),(float)width/height, 0.01f,100.0f);
        shad.set_mat4_uniform("projection",projection);

        //render 1st cube
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(sin((float)glfwGetTime()),0.0f,-5.0f));
        model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1.0f,1.0f,1.0f));
        shad.set_mat4_uniform("model",model);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        //render 2nd cube
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.0f,2.0f,-7.0f));
        model = glm::rotate(model, (float)M_PI/3.0f, glm::vec3(1.0f,1.0f,1.0f));
        shad.set_mat4_uniform("model",model);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);

    glfwTerminate();
    return 0;
}
