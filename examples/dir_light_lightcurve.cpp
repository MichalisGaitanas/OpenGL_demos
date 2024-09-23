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

int main() {
    //initialize glfw and some hints
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //create the window object
    int win_width = 900, win_height = 900;
    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "Directional light calculations", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    //force the window to open at the center of the screen
    int monitx, monity;
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorPos(monitor, &monitx, &monity);
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwSetWindowPos( window, (monitx + mode->width - win_width)/2,
                              (monity + mode->height - win_height)/2 );
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Validate GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        printf("Failed to initialize GLEW. Exiting...\n");
        return 0;
    }

    // Create shader and mesh
    meshvfn sphere("../obj/vfn/uv_sphere_rad1_40x30.obj");
    shader shad("../shaders/vertex/trans_mvpn.vert", "../shaders/fragment/dir_light_ds.frag");
    shad.use();

    glm::vec3 light_dir; //light direction in world coordinates
    glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f); //lighting calculations color
    glm::vec3 sphere_col = glm::vec3(1.0f,1.0f,1.0f); //model color
    glm::vec3 cam_pos = glm::vec3(0.0f,-10.0f,2.0f); //camera position in world coordinates
    glm::vec3 cam_aim = glm::vec3(0.0f,0.0f,0.0f); //camera aim (eye) direction
    glm::vec3 cam_up = glm::vec3(0.0f,0.0f,1.0f); //camera up direction

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)win_width / win_height, 0.01f, 100.0f);
    glm::mat4 view = glm::lookAt(cam_pos, cam_aim, cam_up);
    glm::mat4 model = glm::mat4(1.0f);

    shad.set_mat4_uniform("projection", projection);
    shad.set_mat4_uniform("view", view);
    shad.set_mat4_uniform("model", model);
    shad.set_vec3_uniform("cam_pos", cam_pos);
    shad.set_vec3_uniform("light_col", light_col);
    shad.set_vec3_uniform("model_col", sphere_col);

    // Enable depth testing and set clear color
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // For storing total brightness over time
    float time_step = 0.01f;
    std::vector<float> lightcurve;

    while (!glfwWindowShouldClose(window)) //game loop
    {
        raw_hardware_input(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear off-screen buffer

        light_dir = glm::vec3(cos(glfwGetTime()/3.0), sin(glfwGetTime()/3.0), 0.0f); // Update light direction
        shad.set_vec3_uniform("light_dir", light_dir); // Send updated light direction to the shader
        sphere.draw_triangles(); // Render the sphere to the FBO

        // Step 2: Compute total brightness from the FBO
        //total_brightness += compute_total_brightness(fbo, brightnessTexture, win_width, win_height);
        //frame_count++;

        // Step 3: Render normally to the screen (default framebuffer)
        //glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind the default framebuffer (the screen)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the screen

        sphere.draw_triangles(); // Render the sphere to the screen

        glfwSwapBuffers(window); // Swap the screen buffers
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}