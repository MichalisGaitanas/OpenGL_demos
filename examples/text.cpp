#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<cstdio>
#include"../include/shader.hpp"
#include"../include/font.hpp"

int win_width = 1000, win_height = 700;
const char *win_label = "Text rendering";

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

    GLFWwindow *window = glfwCreateWindow(win_width, win_height, win_label, NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    int monitx, monity;
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorPos(monitor, &monitx, &monity);
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwSetWindowPos( window, (monitx + mode->width - win_width)/2,
                              (monity + mode->height - win_height)/2 );
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //validate glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }

    font ttf("../fonts/NotoSansRegular.ttf"); //instantiate font object
    shader shad("../shaders/vertex/trans_nothing_text.vert","../shaders/fragment/text.frag"); //instatiate shader object for the text
    shad.use();
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(win_width), 0.0f, static_cast<float>(win_height)); //setup text coordinate system
    shad.set_mat4_uniform("projection", projection);

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.05f,0.05f,0.05f,1.0f);
    while (!glfwWindowShouldClose(window))
    {
        raw_hardware_input(window);
        glClear(GL_COLOR_BUFFER_BIT);

        ttf.draw("Hello", 100.0f, 100.0f, win_width, win_height, 1.0f, glm::vec3(0.0f,1.0f,0.5f), shad);
        ttf.draw("Drawing fonts with OpenGL", 500.0f, 400.0f, win_width, win_height, 0.4f, glm::vec3(1.0f,0.0f,0.0f), shad);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
