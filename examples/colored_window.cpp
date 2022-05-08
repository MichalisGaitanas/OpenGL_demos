#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<iostream>

using namespace std;

const int xTopLeft = 300, yTopLeft = 100; //coordinates of the top left corner of the window (y starts from top)
const int winWidth = 900, winHeight = 600;
const char *winLabel = "Colored window";

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

    //create window object
    GLFWwindow *window = glfwCreateWindow(winWidth, winHeight, winLabel, NULL, NULL);
    if (window == NULL)
    {
        cout << "Failed to create glfw window. Exiting...\n";
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowPos(window, xTopLeft,yTopLeft); //control the initial position of the window
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //validate glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize glew. Exiting..." << endl;
        return 0;
    }
                //r   g    b    a
    glClearColor(0.1f,0.4f,0.2f,1.0f); //the background (clear) color for every new frame
    while (!glfwWindowShouldClose(window))
    {
        process_hardware_inputs(window);
        glClear(GL_COLOR_BUFFER_BIT); //actual clearance of the color buffer
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
