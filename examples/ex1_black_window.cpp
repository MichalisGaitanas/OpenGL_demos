#include<GL/glew.h> //Always include this prior to any other OpenGL header.
#include<GLFW/glfw3.h> //Hardware stuff.

#include<cstdio>

int main()
{
    glfwInit(); //Always the first glfw function. It initializes glfw.

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //Version 3.x
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //Version x.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //Core OpenGL.

    //Create window object (at default position).
    GLFWwindow *window = glfwCreateWindow(800, 500, "Black window", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    
    //Glew validation.
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...");
        return 0;
    }

    //Game loop.
    while (!glfwWindowShouldClose(window)) //while(true) (play with it)
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
            //break; (play with it)
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
