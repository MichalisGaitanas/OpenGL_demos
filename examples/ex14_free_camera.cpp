#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<cstdio>

#include"../include/shader.hpp"
#include"../include/mesh.hpp"
#include"../include/camera.hpp"

//Camera object instantiation. We make it global so that the glfw callback 'cursor_pos_callback()' (see later) can
//have access to it. This is just for demo. At a bigger project, we would use glfwSetWindowUserPointer(...) to encapsulate
//any variable within the specific context of the window.
camera cam(glm::vec3(-10.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), 0.0f);

float time_tick; //Elapsed time per frame update.

float xpos_previous, ypos_previous;
bool first_time_entered_the_window = true;

bool cursor_visible = false;

int win_width = 1200, win_height = 900;

//For 'continuous' events, i.e. at every frame (tick) in the while() loop.
void event_tick(GLFWwindow *win)
{
    bool move_key_pressed = false;
    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
    {
        cam.accelerate(time_tick, cam.front);
        move_key_pressed = true;
    }
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
    {
        cam.accelerate(time_tick, -cam.front);
        move_key_pressed = true;
    }
    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
    {
        cam.accelerate(time_tick, cam.right);
        move_key_pressed = true;
    }
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
    {
        cam.accelerate(time_tick, -cam.right);
        move_key_pressed = true;
    }
    if (glfwGetKey(win, GLFW_KEY_E) == GLFW_PRESS)
    {
        cam.accelerate(time_tick, cam.world_up);
        move_key_pressed = true;
    }
    if (glfwGetKey(win, GLFW_KEY_Q) == GLFW_PRESS)
    {
        cam.accelerate(time_tick, -cam.world_up);
        move_key_pressed = true;
    }

    //If no keys are pressed, decelerate.
    if (!move_key_pressed)
        cam.decelerate(time_tick);
}

//For discrete keyboard events.
void key_callback(GLFWwindow *window, int key, int, int action, int)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(window, true);
}

//When a mouse button is pressed, do the following :
void mouse_button_callback(GLFWwindow* window, int button, int action, int)
{
    //Toggle cursor visibility via the mouse right click.
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        cursor_visible = !cursor_visible;
        if (cursor_visible)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            first_time_entered_the_window = true;
        }
    }
}

//When the mouse moves, do the following :
void cursor_pos_callback(GLFWwindow *, double xpos, double ypos)
{
    if (cursor_visible)
        return;

    if (first_time_entered_the_window)
    {
        xpos_previous = xpos;
        ypos_previous = ypos;
        first_time_entered_the_window = false;
    }

    float xoffset = xpos - xpos_previous;
    float yoffset = ypos - ypos_previous;

    xpos_previous = xpos;
    ypos_previous = ypos;

    cam.rotate(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *, double, double yoffset)
{
    if (!cursor_visible)
        cam.zoom((float)yoffset);
}

void framebuffer_size_callback(GLFWwindow *, int w, int h)
{
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    win_width = w;
    win_height = h;
    glViewport(0,0,w,h);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "First person camera", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeLimits(window, 400, 400, GLFW_DONT_CARE, GLFW_DONT_CARE);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //Hide the mouse initially.

    glfwGetWindowSize(window, &win_width, &win_height);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }

    meshvfn sponza_temple("../obj/vfn/sponza_merged.obj");
    shader sponza_shad("../shaders/vertex/trans_mvpn.vert","../shaders/fragment/point_light_ad.frag");

    meshvf sphere_lamp("../obj/vf/uv_sphere_rad1_20x20.obj");
    shader lamp_shad("../shaders/vertex/trans_mvp.vert","../shaders/fragment/monochromatic.frag");

    glm::vec3 sponza_col = glm::vec3(0.5f,0.5f,1.0f);
    glm::vec3 lamp_col = glm::vec3(0.8f,0.8f,1.0f);
    glm::vec3 light_pos = glm::vec3(0.0f,0.0f,3.0f); //Light position in world coordinates.
    glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f); //Light color.

    lamp_shad.use();
    lamp_shad.set_vec3_uniform("mesh_col", lamp_col);

    sponza_shad.use();
    sponza_shad.set_vec3_uniform("mesh_col", sponza_col);
    sponza_shad.set_vec3_uniform("light_pos", light_pos);
    sponza_shad.set_vec3_uniform("light_col", light_col);

    glm::mat4 projection, view, model;

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f,0.1f,0.1f,1.0f);

    float t0 = 0.0f, tnow;
    while (!glfwWindowShouldClose(window)) //Game loop.
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        tnow = (float)glfwGetTime(); //Elapsed time [sec] since glfwInit().
        time_tick = tnow - t0;
        t0 = tnow;

        event_tick(window);

        projection = glm::perspective(glm::radians(cam.fov), (float)win_width/win_height, 0.01f,100.0f);
        model = glm::mat4(1.0f);
        cam.move(time_tick);
        view = cam.view();
        sponza_shad.use();
        sponza_shad.set_mat4_uniform("projection", projection);
        sponza_shad.set_mat4_uniform("view", view);
        sponza_shad.set_mat4_uniform("model", model);
        sponza_temple.draw_triangles();


        model = glm::mat4(1.0f);
        model = glm::translate(model, light_pos);
        lamp_shad.use();
        lamp_shad.set_mat4_uniform("projection", projection);
        lamp_shad.set_mat4_uniform("view", view);
        lamp_shad.set_mat4_uniform("model", model);
        sphere_lamp.draw_triangles();
       
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
