#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<iostream>
#include"../headers/shader.h"

using namespace std;

int winWidth = 900, winHeight = 900;
const char *label = "Point light calculations";

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

    //vertices and normals of the cube
    float geometry[] = {
                            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
                             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
                             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
                             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
                            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
                            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

                            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
                             0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
                             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
                             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
                            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
                            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

                            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
                            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
                            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
                            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
                            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
                            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

                             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
                             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
                             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
                             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
                             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
                             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

                            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
                             0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
                             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
                             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
                            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
                            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

                            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
                             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
                             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
                             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
                            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
                            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    //create cube vao and vbo stuff
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glBindVertexArray(cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), geometry, GL_STATIC_DRAW);
    //vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //normal attributes
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    //create lamp (which is also a cube) vao and vbo stuff
    unsigned int lampVAO, lampVBO;
    glGenVertexArrays(1, &lampVAO);
    glBindVertexArray(lampVAO);
    glGenBuffers(1, &lampVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lampVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), geometry, GL_STATIC_DRAW);
    //only vertex attributes are handled here because we want a single color for the lamp, thus no normals are needed
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //create shaders
    shader cubeShad("../shaders/trans_mvpn.vert","../shaders/point_light_ads.frag");
    shader lampShad("../shaders/trans_mvp.vert","../shaders/monochromatic.frag");

    glEnable(GL_DEPTH_TEST); //enable depth - testing
    glClearColor(0.0f,0.0f,0.0f,1.0f); //background color
    while (!glfwWindowShouldClose(window)) //game loop
    {
        process_hardware_inputs(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 lightPos = glm::vec3(0.0f,-2.5f,1.0f); //light position in world coordinates
        glm::vec3 lightCol = glm::vec3(1.0f,1.0f,1.0f); //lighting calculations color
        glm::vec3 cubeCol = glm::vec3(0.8f,0.0f,0.0f); //model color
        glm::vec3 lampCol = glm::vec3(1.0f,1.0f,1.0f); //lamp color
        glm::vec3 camPos = glm::vec3(-5.0f,-5.0f,1.0f); //camera position in world coordinates

        //winWidth, winHeight are never updated because there's no corresponding "update code" in framebuffer_size_callback()
        glm::mat4 projection = glm::perspective(glm::radians(45.0f),(float)winWidth/(float)winHeight, 0.01f,100.0f);
        glm::mat4 view = glm::lookAt(camPos, glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,1.0f));
        glm::mat4 model = glm::mat4(1.0f);

        cubeShad.use();
        model = glm::rotate(model, (0.5f*(float)glfwGetTime()), glm::vec3(1.0f,1.0f,1.0f));
        cubeShad.set_mat4_uniform("projection", projection);
        cubeShad.set_mat4_uniform("view", view);
        cubeShad.set_mat4_uniform("model", model);
        cubeShad.set_vec3_uniform("lightPos", lightPos);
        cubeShad.set_vec3_uniform("camPos", camPos);
        cubeShad.set_vec3_uniform("lightCol", lightCol);
        cubeShad.set_vec3_uniform("modelCol", cubeCol);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        lampShad.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.5f,0.5f,0.5f));
        lampShad.set_mat4_uniform("projection", projection);
        lampShad.set_mat4_uniform("view", view);
        lampShad.set_mat4_uniform("model", model);
        lampShad.set_vec3_uniform("modelCol", lampCol);
        glBindVertexArray(lampVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteVertexArrays(1, &lampVAO);
    glDeleteBuffers(1, &lampVBO);

    glfwTerminate();
    return 0;
}
