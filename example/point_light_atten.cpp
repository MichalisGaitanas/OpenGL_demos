#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<cstdio>
#include<cmath>
#include"../include/shader.hpp"
#include"../include/mesh.hpp"

void raw_hardware_input(GLFWwindow *win)
{
    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(win, true);
    }
}

void framebuffer_size_callback(GLFWwindow *win, int w, int h)
{
    glViewport(0,0,w,h);
}

int main()
{
    //initialize glfw and some hints
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE); //full screen (windowed)

    GLFWwindow *window = glfwCreateWindow(500, 500, "Point light with attenuation", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); //set framebuffer size callback function

    //calculate window dimensions
    int win_width, win_height;
    glfwGetWindowSize(window, &win_width, &win_height);

    //validate glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }

    //create meshes
    mesh suzanne("../obj/vert_face_snorm/suzanne.obj", 1,1,1);
    mesh lamp("../obj/vert_face_snorm/sphere_rad1.obj", 1,1,0);

    //create shaders
    shader suzanne_shad("../shader/vertex/trans_mvpn.vert","../shader/fragment/point_light_ads_atten.frag");
    shader lamp_shad("../shader/vertex/trans_mvp.vert","../shader/fragment/monochromatic.frag");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.05f,0.05f,0.05f,1.0f);
    while (!glfwWindowShouldClose(window))
    {
        raw_hardware_input(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 light_pos = glm::vec3(0.0f, -5.0f + 3.0f*sin(glfwGetTime()), 0.0f); //light position in world coordinates
        glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f); //lighting calculations color
        glm::vec3 model_col = glm::vec3(0.8f,0.4f,0.0f); //suzanne color
        glm::vec3 lamp_col = glm::vec3(1.0f,1.0f,1.0f); //lamp color
        glm::vec3 cam_pos = glm::vec3(0.0f,-15.0f,7.5f); //camera position in world coordinates

        glm::mat4 projection = glm::perspective(glm::radians(45.0f),(float)win_width/(float)win_height, 0.01f,100.0f);
        glm::mat4 view = glm::lookAt(cam_pos, glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,1.0f));
        glm::mat4 model = glm::mat4(1.0f);

        suzanne_shad.use();
        suzanne_shad.set_mat4_uniform("projection", projection);
        suzanne_shad.set_mat4_uniform("view", view);
        suzanne_shad.set_mat4_uniform("model", model);
        suzanne_shad.set_vec3_uniform("light_pos", light_pos);
        suzanne_shad.set_vec3_uniform("cam_pos", cam_pos);
        suzanne_shad.set_vec3_uniform("light_col", light_col);
        suzanne_shad.set_vec3_uniform("model_col", model_col);
        suzanne.draw_triangles();

        lamp_shad.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, light_pos);
        model = glm::scale(model, glm::vec3(0.25f,0.25f,0.25f));
        lamp_shad.set_mat4_uniform("projection", projection);
        lamp_shad.set_mat4_uniform("view", view);
        lamp_shad.set_mat4_uniform("model", model);
        lamp_shad.set_vec3_uniform("model_col", lamp_col);
        lamp.draw_triangles();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
