#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<iostream>

using namespace std;

int winWidth = 1000, winHeight = 700;
const char *winLabel = "Centered window";

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
    //initialize glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //create the window object
    GLFWwindow *window = glfwCreateWindow(winWidth, winHeight, winLabel, NULL, NULL);
    if (window == NULL)
    {
        cout << "Failed to create glfw window. Exiting...\n";
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);

    //force the window to open at the center of the screen
    int monitx, monity; //monitor's top left corner coordinates in the virtual space
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorPos(monitor, &monitx, &monity);
    cout << "Monitor's coordinates in virtual space : (" << monitx << "," << monity << ")\n";
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwSetWindowPos( window, (monitx + mode->width - winWidth)/2,
                              (monity + mode->height - winHeight)/2 );

    //register framebuffer callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //validate glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize glew. Exiting..." << endl;
        return 0;
    }

    glClearColor(0.1f,0.4f,0.8f,1.0f); //set background - clear color for every new frame
    while (!glfwWindowShouldClose(window))
    {
        process_hardware_inputs(window);
        glClear(GL_COLOR_BUFFER_BIT); //actually clear the buffer using the predefined clear color
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
