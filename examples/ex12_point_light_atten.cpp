#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include<cstdio>
#include<cmath>

#include"../include/shader.hpp"
#include"../include/mesh.hpp"

int win_width = 1500, win_height = 900;

//When a keyboard key is pressed :
void key_callback(GLFWwindow *window, int key, int, int action, int)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(window, true);
}

//When the framebuffer is resized :
void framebuffer_size_callback(GLFWwindow *, int w, int h)
{
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    win_width = w;
    win_height = h;
    glViewport(0,0,w,h);
}

//Center the window.
void glfw_center_window(GLFWwindow *win)
{
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwGetWindowSize(win, &win_width, &win_height);
    int centx = (mode->width - win_width)/2;
    int centy = (mode->height - win_height)/2;
    glfwSetWindowPos(win, centx, centy);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "Point light with attenuation", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    glfw_center_window(window);
    
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...");
        return 0;
    }

    //Create Meshes.
    meshvfn suzanne("../obj/vfn/suzanne.obj");
    meshvf lamp("../obj/vf/uv_sphere_rad1_20x20.obj");

    //Create shaders.
    shader suzanne_shad("../shaders/vertex/trans_mvpn.vert","../shaders/fragment/point_light_ads_atten.frag");
    shader lamp_shad("../shaders/vertex/trans_mvp.vert","../shaders/fragment/monochromatic.frag");

    glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f);
    glm::vec3 lamp_col = glm::vec3(1.0f,1.0f,1.0f);
    glm::vec3 suzanne_col = glm::vec3(0.8f,0.4f,0.0f);
    glm::vec3 cam_pos = glm::vec3(0.0f,15.0f,9.0f);
    glm::vec3 lamp_pos;

    glm::mat4 projection, view, model;
    view = glm::lookAt(cam_pos, glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,1.0f));

    lamp_shad.use();
    lamp_shad.set_vec3_uniform("mesh_col", lamp_col);
    lamp_shad.set_mat4_uniform("view", view);

    suzanne_shad.use();
    suzanne_shad.set_vec3_uniform("mesh_col", suzanne_col);
    suzanne_shad.set_vec3_uniform("light_col", light_col);
    suzanne_shad.set_vec3_uniform("cam_pos", cam_pos);
    suzanne_shad.set_mat4_uniform("view", view);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.05f,0.05f,0.05f,1.0f);
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        lamp_pos = glm::vec3(0.0f, 5.0f + 3.0f*sin(glfwGetTime()), 0.0f); //light position in world coordinates

        projection = glm::perspective(glm::radians(45.0f),(float)win_width/(float)win_height, 0.01f,100.0f);

        suzanne_shad.use();
        model = glm::mat4(1.0f);
        suzanne_shad.set_mat4_uniform("projection", projection);
        suzanne_shad.set_mat4_uniform("model", model);
        suzanne_shad.set_vec3_uniform("light_pos", lamp_pos);
        suzanne.draw_triangles();

        lamp_shad.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, lamp_pos);
        model = glm::scale(model, glm::vec3(0.25f,0.25f,0.25f));
        lamp_shad.set_mat4_uniform("projection", projection);
        lamp_shad.set_mat4_uniform("model", model);
        lamp_shad.set_vec3_uniform("light_pos", lamp_pos);
        lamp.draw_triangles();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
