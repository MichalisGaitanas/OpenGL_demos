#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<iostream>
#include<cmath>

#include"../headers/shader.h"
#include"../headers/mesh.h"
#include"../headers/camera.h"

using namespace std;

camera cam(glm::vec3(0.0f,-10.0f,0.0f)); //global camera object

float t1 = 0.0f, t2, deltaTime;
float xposPrevious, yposPrevious;
bool firstTimeEnteredTheWindow = true;

void process_hardware_inputs(GLFWwindow *win)
{
    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(win, true);

    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
        cam.translate_front(deltaTime);
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
        cam.translate_back(deltaTime);
    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
        cam.translate_right(deltaTime);
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
        cam.translate_left(deltaTime);
}

void cursor_pos_callback(GLFWwindow *win, double xpos, double ypos)
{
    if (firstTimeEnteredTheWindow)
    {
        xposPrevious = xpos;
        yposPrevious = ypos;
        firstTimeEnteredTheWindow = false;
    }

    float xoffset = xpos - xposPrevious;
    float yoffset = ypos - yposPrevious;

    xposPrevious = xpos;
    yposPrevious = ypos;

    cam.rotate(xoffset, yoffset);
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
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow *window = glfwCreateWindow(900, 900, "Flashlight", NULL, NULL);
    if (window == NULL)
    {
        cout << "Failed to create glfw window. Exiting...\n";
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    int winWidth, winHeight;
    glfwGetWindowSize(window, &winWidth, &winHeight);
    cout << "Window size (in screen coordinates) = " << winWidth << ", " << winHeight << endl;
    xposPrevious = winWidth/2.0f;
    yposPrevious = winHeight/2.0f;

    //validate glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize glew. Exiting...\n";
        return 0;
    }

    mesh suzanne("../models/smooth/suzanne.obj", 1,1,1);
    mesh tree("../models/flat/low_poly_tree.obj", 1,1,1);
    mesh man("../models/smooth/man.obj",1,1,1);
    shader shad("../shaders/trans_mvpn.vert","../shaders/flashlight_ad.frag"); //create shader

    glm::mat4 projection = glm::perspective(glm::radians(45.0f),(float)winWidth/winHeight, 0.01f,1000.0f);
    glm::mat4 view, model;

    glm::vec3 lightPos; //light position in world coordinates
    glm::vec3 lightCol = glm::vec3(1.0f,1.0f,1.0f); //lighting calculations color
    glm::vec3 modelCol = glm::vec3(0.5f,0.5f,0.5f); //color of the cuboids
    float cosCuttoff = cos(glm::radians(10.0f));

    shad.use();
    shad.set_mat4_uniform("projection", projection);
    shad.set_vec3_uniform("lightCol", lightCol);
    shad.set_vec3_uniform("modelCol", modelCol);
    shad.set_float_uniform("cosCuttoff", cosCuttoff);

    glEnable(GL_DEPTH_TEST); //enable depth - testing
    glClearColor(0.1f,0.1f,0.1f,1.0f); //background color
    while (!glfwWindowShouldClose(window)) //game loop
    {
        t2 = glfwGetTime(); //elased time [sec] since glfwInit()
        deltaTime = t2 - t1;
        t1 = t2;

        process_hardware_inputs(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //color buffer and z-buffer

        model = glm::mat4(1.0f);
        view = cam.view();
        lightPos = cam.pos;
        shad.set_mat4_uniform("model", model);
        shad.set_mat4_uniform("view", view);
        shad.set_vec3_uniform("camPos", cam.pos);
        shad.set_vec3_uniform("camFront", cam.front);
        shad.set_vec3_uniform("lightPos",lightPos);
        tree.draw_triangles(); //draw the tree at the default model position (model matrix = 1)

        model = glm::translate(model,glm::vec3(4.0f,0.0f,0.0f)); //update the model matrix
        shad.set_mat4_uniform("model", model); //inform the shader
        suzanne.draw_triangles(); //draw suzanne at the new (translated) position

        model = glm::mat4(1.0f); //set matrix = 1 so that the next operation begins from identity matrix
        model = glm::translate(model,glm::vec3(-4.0f,0.0f,0.0f)); //update the model matrix
        shad.set_mat4_uniform("model", model); //draw the man at the new (translated) position.
        man.draw_triangles();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
