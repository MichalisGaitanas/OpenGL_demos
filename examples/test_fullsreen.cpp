#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<iostream>
#include"../headers/shader.h"
#include"../headers/mesh.h"
#include"../headers/font.h"

using namespace std;

int winWidth, winHeight;
float aspectRatio;
bool fullscreen = false; //not at fullscreen initially

void process_hardware_inputs(GLFWwindow *win)
{
    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(win, true);
    }
}

void framebuffer_size_callback(GLFWwindow *win, int w, int h)
{
    cout << winWidth << " " << winHeight <<  endl;
    //make sure the window will not crash in case of "akward" resize
    if (w == 0) w = 1;
    if (h == 0) h = 1;
    glViewport(0,0,w,h);
    winWidth = w;
    winHeight = h;
    aspectRatio = (float)w/h;
}

void key_callback(GLFWwindow *win, int key, int scancode, int action, int mods)
{

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    if (glfwGetKey(win, GLFW_KEY_F) == GLFW_PRESS)
    {
        fullscreen = !fullscreen;
    }
    if (fullscreen)
    {
        glfwSetWindowMonitor(win, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    }
    else
    {
        glfwSetWindowMonitor(win, NULL, (mode->width - winWidth)/2, (mode->height - winHeight)/2, winWidth, winHeight, mode->refreshRate);
        glfwSetWindowSize(win, 800, 600);
    }
}

int main()
{
    //initialize glfw and some hints
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //create the window object
    winWidth = 800;
    winHeight = 600;
    aspectRatio = (float)winWidth/winHeight;
    GLFWwindow *window = glfwCreateWindow(winWidth, winHeight, "Toggle fullscreen", NULL, NULL);
    if (window == NULL)
    {
        cout << "Failed to create glfw window. Exiting...\n";
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);

    //force the window to open at the center of the screen
    int monitx, monity;
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorPos(monitor, &monitx, &monity);
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwSetWindowPos( window, (monitx + mode->width - winWidth)/2, (monity + mode->height - winHeight)/2 );
    glfwSetWindowSizeLimits(window, 200, 200, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    //validate glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize glew. Exiting...\n";
        return 0;
    }

    //create mesh and corresponding shader
    mesh suzanne("../models/smooth/suzanne.obj",1,1,1);
    shader suzanneShader("../shaders/trans_mvpn.vert","../shaders/dir_light_ads.frag");
    //create font and corresponding shader
    font ttf("../fonts/NotoSansRegular.ttf");
    shader textShader("../shaders/text.vert","../shaders/text.frag");

    glm::vec3 lightDir = glm::vec3(1.0f,1.0f,1.0f);
    glm::vec3 lightCol = glm::vec3(1.0f,1.0f,1.0f);
    glm::vec3 suzanneCol = glm::vec3(0.3f,0.4f,0.1f);
    glm::vec3 camPos = glm::vec3(8.0f,8.0f,0.0f);
    glm::vec3 camAim = glm::vec3(0.0f,0.0f,0.0f);
    glm::vec3 camUp = glm::vec3(0.0f,0.0f,1.0f);
    glm::mat4 view = glm::lookAt(camPos, camAim, camUp); //this is constant because the camera is still
    suzanneShader.use();
    suzanneShader.set_vec3_uniform("lightDir", lightDir);
    suzanneShader.set_vec3_uniform("lightCol", lightCol);
    suzanneShader.set_vec3_uniform("modelCol", suzanneCol);
    suzanneShader.set_vec3_uniform("camPos", camPos);
    suzanneShader.set_mat4_uniform("view", view);

    glm::mat4 projection; //this changes in case of framebuffer resize, hence we update it in the render loop
    glm::mat4 model; //this also changes in the render loop because the model rotates

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.1f,0.1f,0.1f,1.0f);
    while (!glfwWindowShouldClose(window))
    {
        process_hardware_inputs(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //draw suzanne
        projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
        model = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.0f,0.0f,1.0f));
        suzanneShader.use();
        suzanneShader.set_mat4_uniform("projection", projection);
        suzanneShader.set_mat4_uniform("model", model);
        suzanne.draw_triangles();

        //draw text
        char text[50];
        sprintf(text, "Time info : t = %.2f [sec]",(float)glfwGetTime());
        ttf.draw(text, 20.0f, 50.0f, winWidth, winHeight, 0.4f, glm::vec3(1.0f,1.0f,1.0f), textShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
