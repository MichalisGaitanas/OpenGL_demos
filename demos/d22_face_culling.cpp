#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<cstdio>

#include"../include/shader.h"
#include"../include/mesh.h"
#include"../include/camera.h"

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
void key_callback(GLFWwindow *window, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(window, true);
}

//When a mouse button is pressed, do the following :
void mouse_button_callback(GLFWwindow *window, int button, int action, int /*mods*/)
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
void cursor_pos_callback(GLFWwindow */*win*/, double xpos, double ypos)
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

void scroll_callback(GLFWwindow */*win*/, double /*xoffset*/, double yoffset)
{
    cam.zoom((float)yoffset);
}

void framebuffer_size_callback(GLFWwindow */*win*/, int w, int h)
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

    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "Face culling", NULL, NULL);
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
    ImGui_ImplOpenGL3_Init("#version 450");
    ImGuiStyle &imstyle = ImGui::GetStyle();
    imstyle.WindowMinSize = ImVec2(100.0f,100.0f);
    imstyle.FrameRounding = 5.0f;
    imstyle.WindowRounding = 5.0f;

    meshvfn sponza("../obj/vfn/sponza_merged.obj");
    meshvfn sphere("../obj/vfn/uv_sphere_rad1_40x30.obj");

    shader shad("../shaders/vertex/trans_mvpn.vert","../shaders/fragment/dir_light_ads.frag");
    shad.use(); //We only have 1 shader, so we activate it here once and for all.

    glm::vec3 mesh_col = glm::vec3(0.8f,0.0f,0.0f);
    glm::vec3 light_dir = glm::vec3(1.0f,1.0f,1.0f);
    glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f);
    shad.set_vec3_uniform("mesh_col", mesh_col);
    shad.set_vec3_uniform("light_dir", light_dir);
    shad.set_vec3_uniform("light_col", light_col);

    glm::mat4 projection, view, model;

    glEnable(GL_DEPTH_TEST);
    
    glEnable(GL_CULL_FACE); //Enable face culling.
    glCullFace(GL_BACK); ////Which face to cull.
    glFrontFace(GL_CCW); //Which face to assume as front.
    bool face_cull_is_enabled = true;
    bool front_face_is_ccw = true;

    glClearColor(0.1f,0.1f,0.1f,1.0f);

    float t0 = 0.0f, tnow;

    while (!glfwWindowShouldClose(window)) //Game loop.
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //The depth buffer will be cleared in accordance with the argument passed in glDepthFunc().
        
        tnow = (float)glfwGetTime(); //Elapsed time [sec] since glfwInit().
        time_tick = tnow - t0;
        t0 = tnow;

        event_tick(window); 
        
        //Enable/disable face culling.
        if (face_cull_is_enabled)
            glEnable(GL_CULL_FACE);
        else
            glDisable(GL_CULL_FACE);

        //Choose which side of the face will be considered front or back in the culling protocol.
        if (front_face_is_ccw)
            glFrontFace(GL_CCW);
        else
            glFrontFace(GL_CW);

        projection = glm::perspective(glm::radians(cam.fov), (float)win_width/win_height, 0.01f,500.0f);
        cam.move(time_tick);
        view = cam.view();
        shad.set_mat4_uniform("projection", projection);
        shad.set_vec3_uniform("cam_pos", cam.pos);
        shad.set_mat4_uniform("view", view);

        model = glm::mat4(1.0f);
        shad.set_mat4_uniform("model", model);
        sponza.draw_triangles();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f,0.0f,50.0f));
        model = glm::scale(model, glm::vec3(5.0f,5.0f,5.0f));
        shad.set_mat4_uniform("model", model);
        sphere.draw_triangles();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(300.0f,200.0f), ImGuiCond_FirstUseEver); 
        static bool closable = true;
		ImGui::Begin("GUI", &closable);
        if (!closable)
            glfwSetWindowShouldClose(window, true);
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0,0,120,255));
        ImGui::Checkbox("Face cull is enabled", &face_cull_is_enabled);
        ImGui::Checkbox("Front face is ccw", &front_face_is_ccw);
        ImGui::PopStyleColor();
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
