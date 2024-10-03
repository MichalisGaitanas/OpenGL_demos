#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"


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
camera cam(glm::vec3(-15.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 1.0f), 0.0f);

float time_tick; //Elapsed time per frame update.

float xpos_previous, ypos_previous;
bool first_time_entered_the_window = true;

bool cursor_visible = false;

int win_width = 1200, win_height = 900;

//For 'continuous' events, i.e. at every frame in the while() loop.
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
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(window, true);
}

//When a mouse button is pressed, do the following :
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
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
void cursor_pos_callback(GLFWwindow *win, double xpos, double ypos)
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

void scroll_callback(GLFWwindow *win, double xoffset, double yoffset)
{
    cam.zoom((float)yoffset);
}

void framebuffer_size_callback(GLFWwindow *win, int w, int h)
{
    win_width = w;
    win_height = h;
    glViewport(0,0,w,h);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "Depth buffer", NULL, NULL);
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

    //Setup ImGui.
	IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;
    io.Fonts->AddFontFromFileTTF("../fonts/Arial.ttf", 15.0f);
    (void)io;
    ImGui::StyleColorsLight();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGuiStyle &imstyle = ImGui::GetStyle();
    imstyle.WindowMinSize = ImVec2(100.0f,100.0f);
    imstyle.FrameRounding = 5.0f;
    imstyle.WindowRounding = 5.0f;

    meshvf sponza_palace("../obj/vf/sponza_merged.obj");

    shader shad_depth_default("../shaders/vertex/trans_mvp.vert","../shaders/fragment/depth_buffer.frag");
    shader shad_depth_linear("../shaders/vertex/trans_mvp.vert","../shaders/fragment/depth_buffer_linear.frag");
    bool use_linear_depth_shader = true;

    glm::mat4 projection, view, model;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); //Default algorithm for the depth testing, but let's highlight it.
    glClearColor(0.1f,0.1f,0.1f,1.0f);

    float t1 = 0.0f, t2;

    while (!glfwWindowShouldClose(window)) //game loop
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //The depth buffer will be cleared in accordance with the argument passed in glDepthFunc().
        
        t2 = (float)glfwGetTime(); //Elapsed time [sec] since glfwInit().
        time_tick = t2 - t1;
        t1 = t2;

        event_tick(window);

        projection = glm::perspective(glm::radians(cam.fov), (float)win_width/win_height, 0.01f,500.0f);
        cam.move(time_tick);
        view = cam.view();
        model = glm::mat4(1.0f);
        
        if (use_linear_depth_shader)
        {
            shad_depth_linear.use();
            shad_depth_linear.set_mat4_uniform("projection", projection);
            shad_depth_linear.set_mat4_uniform("view", view);
            shad_depth_linear.set_mat4_uniform("model", model);
        }
        else
        {
            shad_depth_default.use();
            shad_depth_default.set_mat4_uniform("projection", projection);
            shad_depth_default.set_mat4_uniform("view", view);
            shad_depth_linear.set_mat4_uniform("model", model);
        }
        sponza_palace.draw_triangles();

        //Render the GUI window so that the user may toggle between the 2 shaders.

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(300.0f,200.0f), ImGuiCond_FirstUseEver); 
        static bool closable = true;
		ImGui::Begin("GUI", &closable);
        if (!closable)
            glfwSetWindowShouldClose(window, true);
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0,100,0,255));
        ImGui::Text("Right click to toggle the cursor.");
        ImGui::PopStyleColor();
        ImGui::Checkbox("Use linear depth shader", &use_linear_depth_shader);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}
