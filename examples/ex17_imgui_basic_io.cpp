
#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<cstdio>

void key_callback(GLFWwindow *window, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow */*window*/, int width, int height)
{
    glViewport(0,0,width,height);
}

int main()
{
	glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800,700, "Basic imgui io", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to open a glfw window. Exiting...\n");
        return 0;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }

    //Initialize imgui.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;
    io.Fonts->AddFontFromFileTTF("../fonts/Arial.ttf", 15.0f);
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGuiStyle &imstyle = ImGui::GetStyle();
    imstyle.WindowMinSize = ImVec2(100.0f,100.0f);
    imstyle.FrameRounding = 5.0f;
    imstyle.WindowRounding = 5.0f;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    while (!glfwWindowShouldClose(window))
    {
		glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos( ImVec2(0.0f, 0.0f), ImGuiCond_FirstUseEver);
        //ImGui::SetNextWindowSize(ImVec2(300.0f, 600.0f), ImGuiCond_FirstUseEver);
        static bool popen = true;
        ImGui::Begin("Basic io", &popen); //Imgui window with title and a close button.
        if (!popen)
            glfwSetWindowShouldClose(window, true);

        ImGui::Text("Text printing"); //Raw text printing on the gui.
        
        ImGui::Dummy(ImVec2(0.0f, 20.0f)); //Vertical space (dx,dy).

        static char buf[30] = "";
        ImGui::PushItemWidth(100.0f); //Change item's width from default to a fixed one.
            ImGui::InputText("Input text", buf, IM_ARRAYSIZE(buf)); //Input section.
        ImGui::PopItemWidth(); //Reset the item's width from the fixed one we set to the default one.
        
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        //ImGui::Separator(); //Horizontal decoration-seperation line.

        static float x = 0.12345f;
        ImGui::Text("Enter a float");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(0); //Define a new ID for the upcoming float, so it does not confilct with next inputs,
                ImGui::InputFloat(" ", &x, 0.0f, 0.0f,"%.3f"); //Float value input.
            ImGui::PopItemWidth();
        ImGui::PopID();
        
        //ImGui::Separator();

        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        static bool action = true;
        ImGui::Checkbox("Action", &action); //Checkbox.
        ImGui::SameLine();
        if (action)
            ImGui::Text("(Checked)");
        else
            ImGui::Text("(Unchecked)");

        //Classical buttons.
        if (ImGui::Button("Button 1")) { /* Do stuff. */ }
        ImGui::SameLine();
        if (ImGui::Button("Button 2")) { /* Do stuff. */ }
        ImGui::SameLine();
        if (ImGui::Button("Button 3")) { /* Do stuff. */ }
        
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255)); //Change color from default to custom RGBA.
        ImGui::Text("Let's create a huge button");
        ImGui::PopStyleColor(); //Reset the last style color that was altered.

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Button("XXX", ImVec2(150,70)); //Classical button and its size (dx,dy).
        ImGui::PopStyleColor(3); //Reset the last 3 style colors that were altered.
        
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        ImGui::Text("Drop-down menu (combo)");
        static const char *items[] = {"Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune"};
        static int selected = 0;
        ImGui::PushItemWidth(100.0f);
            ImGui::Combo("Planet", &selected, items, IM_ARRAYSIZE(items)); //Drop down menu.
        ImGui::PopItemWidth(); //Set item width back to default.
        
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        static int y = 0;
        ImGui::Text("Enter an int");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(1); //Define a new ID for the upcoming int, so it does not confilct with the previous float (x).
                ImGui::InputInt(" ", &y, 0,0);
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        ImGui::BulletText("Bullet text demo");

        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        static double M = 1e-13;
        ImGui::Text("Mass");
        ImGui::Text("M ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(2);
                ImGui::InputDouble("", &M, 0.0, 0.0, "%g");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[kg]");

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //Free imgui resources.
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();

	return 0;
}