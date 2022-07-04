#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<cstdio>

const int x_top_left = 300, y_top_left = 100; //coordinates of the top left corner of the window (y starts from top)
const int win_width = 900, win_height = 600;
const char *win_label = "Colored window";

void raw_hardware_input(GLFWwindow *window)
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
    GLFWwindow *window = glfwCreateWindow(win_width, win_height, win_label, NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowPos(window, x_top_left,y_top_left); //control the initial position of the window
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //validate glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...");
        return 0;
    }
                //r   g    b    a
    glClearColor(0.1f,0.4f,0.2f,1.0f); //the background (clear) color for every new frame
    while (!glfwWindowShouldClose(window))
    {
        raw_hardware_input(window);
        glClear(GL_COLOR_BUFFER_BIT); //actual clearance of the color buffer
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
