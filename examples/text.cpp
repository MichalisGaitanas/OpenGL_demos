#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<iostream>
#include"../headers/shader.h"
#include"../headers/font.h"

using namespace std;

int winWidth = 1000, winHeight = 700;
const char *winLabel = "Text rendering";

void process_hardware_inputs(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0,0,width,height);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(winWidth, winHeight, winLabel, NULL, NULL);
    if (window == NULL)
    {
        cout << "Failed to create glfw window. Exiting...\n";
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
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

    font ttf("../fonts/NotoSansRegular.ttf"); //instantiate font object
    shader shad("../shaders/text.vert","../shaders/text.frag"); //instatiate shader object for the text
    shad.use();
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(winWidth), 0.0f, static_cast<float>(winHeight)); //setup text coordinate system
    shad.set_mat4_uniform("projection", projection);

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.05f,0.05f,0.05f,1.0f);
    while (!glfwWindowShouldClose(window))
    {
        process_hardware_inputs(window);
        glClear(GL_COLOR_BUFFER_BIT);

        ttf.draw("Hello", 100.0f, 100.0f, winWidth, winHeight, 1.0f, glm::vec3(0.0f,1.0f,0.5f), shad);
        ttf.draw("Drawing fonts with OpenGL", 500.0f, 400.0f, winWidth, winHeight, 0.4f, glm::vec3(1.0f,0.0f,0.0f), shad);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
