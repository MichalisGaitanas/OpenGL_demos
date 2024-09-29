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

    int win_width = 1000, win_height = 700;
    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "Centered window", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);

    //Force the window to open at the center of the screen.
    int monitx, monity; //Monitor's top left corner coordinates in the virtual space.
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorPos(monitor, &monitx, &monity);
    printf("Monitor's coordinates in virtual space : (%d, %d)\n",monitx,monity);
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwSetWindowPos( window, (monitx + mode->width - win_width)/2,
                              (monity + mode->height - win_height)/2 );

    glfwSetKeyCallback(window, key_callback);

    //validate glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...");
        return 0;
    }

    glClearColor(0.1f,0.4f,0.8f,1.0f);
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
