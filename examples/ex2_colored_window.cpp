#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<cstdio>

void key_callback(GLFWwindow *win, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
    {
        glfwSetWindowShouldClose(win, true);
    }
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //Create window object (pointer to struct).
    GLFWwindow *window = glfwCreateWindow(900, 600, "Colored window", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    
    int x_top_left = 400, y_top_left = 100; //Coordinates of the top left corner of the window (y starts from top).
    glfwSetWindowPos(window, x_top_left, y_top_left); //Control the initial position of the window.
    
    glfwSetKeyCallback(window, key_callback); //Register the keyboard callback function.

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...");
        return 0;
    }
                //r   g    b    a
    glClearColor(0.1f,0.4f,0.2f,1.0f); //The background (clear) color for every new frame.
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT); //Actual clearance of the color buffer.
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
