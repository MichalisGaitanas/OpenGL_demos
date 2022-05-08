#include<GL/glew.h> //always include this prior to any other OpenGL header
#include<GLFW/glfw3.h> //window and hardware stuff
#include<iostream>

using namespace std;

//keyborad, mouse, etc...
void process_hardware_inputs(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

//in case of window resize, this function will be executed
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0,0,width,height); //where to render inside the current active window
}

int main()
{
    glfwInit(); //always the first glfw function
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //version 3.x
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //version x.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //core OpenGL
    //create window object (at default position)
    GLFWwindow *window = glfwCreateWindow(800, 600, "Black window", NULL, NULL);
    if (window == NULL)
    {
        cout << "Failed to create glfw window. Exiting...\n";
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); //register the callback
    //glew validation stuff...
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize glew. Exiting..." << endl;
        return 0;
    }

    //render loop
    while (!glfwWindowShouldClose(window))
    {
        process_hardware_inputs(window); //keyboard, mouse, etc...
        glfwSwapBuffers(window); //double buffering
        glfwPollEvents(); //poll events
    }

    glfwTerminate();
    return 0;
}
