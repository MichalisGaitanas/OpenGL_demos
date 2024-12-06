#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include<cstdio>
#include<cmath>

#include"../include/shader.h"
#include"../include/mesh.h"

int win_width = 1500, win_height = 900;

void key_callback(GLFWwindow *window, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow */*win*/, int w, int h)
{
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    win_width = w;
    win_height = h;
    glViewport(0,0,w,h);
}

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

    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "Texture scene", NULL, NULL);
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
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }

    //Load the meshes with the corresponding textures.
    meshvft ground("../obj/vft/plane10x10.obj", "../images/texture/aerial_grass_rock_diff_4k.jpg");
    meshvft wooden_stool("../obj/vft/wooden_stool.obj", "../images/texture/wooden_stool_diff_2k.jpg");
    meshvft brick_cube("../obj/vft/cube1x1x1_correct_uv.obj", "../images/texture/red_brick_diff_2k.jpg");
    meshvft wooden_container("../obj/vft/cube1x1x1_correct_uv.obj", "../images/texture/wooden_container_diff_512x512.jpg");
    meshvft plant_pot("../obj/vft/plant_pot.obj", "../images/texture/potted_plant_pot_diff_2k.png");
    meshvft plant_leaves("../obj/vft/plant_leaves.obj", "../images/texture/potted_plant_leaves_diff_2k.png");

    shader texshad("../shaders/vertex/trans_mvp_texture.vert","../shaders/fragment/texture.frag");
    texshad.use();

    glm::mat4 projection, view, model;

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f,0.7f,1.0f,1.0f);
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        projection = glm::perspective(glm::radians(45.0f), (float)win_width/(float)win_height, 0.01f,100.0f);
        view = glm::lookAt(glm::vec3(5.0f*(float)cos(0.1f*glfwGetTime()),5.0f*(float)sin(0.1f*glfwGetTime()),2.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,2.0f));
        texshad.set_mat4_uniform("projection", projection);
        texshad.set_mat4_uniform("view", view);

        //Ground :
        model = glm::mat4(1.0f);
        texshad.set_mat4_uniform("model", model);
        ground.draw_triangles();

        //Wooden stool :
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f,0.0f,0.0f));
        texshad.set_mat4_uniform("model", model);
        wooden_stool.draw_triangles();

        //Brick cube :
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0f,0.5f,0.5f));
        texshad.set_mat4_uniform("model", model);
        brick_cube.draw_triangles();

        //Wooden container :
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f,-0.8f,0.5f));
        texshad.set_mat4_uniform("model", model);
        wooden_container.draw_triangles();

        //Plant (pot and leaves) :
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.7f,0.7f,0.0f)); //Redundant...
        texshad.set_mat4_uniform("model", model);
        plant_pot.draw_triangles();
        plant_leaves.draw_triangles();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
