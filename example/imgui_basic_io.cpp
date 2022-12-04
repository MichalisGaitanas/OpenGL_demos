#include"imgui.h"
#include"imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<cstdio>

void raw_hardware_input(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    return;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0,0,width,height);
    return;
}

int main()
{
	glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800,600, "Basic io", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to open a glfw window. Exiting...\n");
        return 0;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    while (!glfwWindowShouldClose(window))
    {
		glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Basic io");

        ImGui::Text("Text printing"); //raw text printing on the gui
        ImGui::Dummy(ImVec2(0.0f, 20.0f)); //vertical space (dx,dy)

        static char buf[30] = "";
        ImGui::PushItemWidth(100.0f); //change item-widget width from default to a fixed one (w)
        ImGui::InputText("Input text", buf, IM_ARRAYSIZE(buf)); //input section
        ImGui::PopItemWidth(); //change item-widget width from default to a fixed one (w)
        ImGui::Dummy(ImVec2(0.0f, 20.0f)); //vertical space space (dx,dy)

        ImGui::Separator(); //decoration-seperation line

        static float x = 0.12345f;
        ImGui::PushID(0); //define a new ID for the upcoming float, so it does not confilct with next inputs
        ImGui::Text("Enter float"); ImGui::SameLine(); ImGui::InputFloat(" ", &x, 0.0f, 0.0f,"%.3f"); //float value input
        ImGui::PopID();
        ImGui::Separator(); //decoration-seperation line
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        static bool action = true;
        ImGui::Checkbox("Action", &action); //checkbox
        ImGui::SameLine();
        if (action) ImGui::Text("(checked)");
        else ImGui::Text("(unchecked)");

        //classical buttons
        if (ImGui::Button("Button 1")) { /* do stuff */ }
        ImGui::SameLine();
        if (ImGui::Button("Button 2")) { /* do stuff */ }
        ImGui::SameLine();
        if (ImGui::Button("Button 3")) { /* do stuff */ }
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255)); //change color from default to custom RGBA
        ImGui::Text("Let's create a huge button");
        ImGui::PopStyleColor(); //set color back to default
        ImGui::Button("X", ImVec2(150,70)); //classical button + its size (dx,dy)
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        ImGui::Text("Drop-down menu (combo)");
        static const char *items[] = {"Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune"};
        static int selected = 0;
        ImGui::PushItemWidth(100.0f); //change item-widget width from default to a fixed one (w)
        ImGui::Combo("Planet", &selected, items, IM_ARRAYSIZE(items)); //drop down menu
        ImGui::PopItemWidth(); //set item width back to default
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        static int y = 0;
        ImGui::PushID(1); //define a new ID for the upcoming int, so it does not confilct with the previous float
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("Enter int"); ImGui::SameLine(); ImGui::InputInt(" ", &y, 0,0); //int value input
        ImGui::PopID();
        ImGui::PopItemWidth();

        ImGui::Dummy(ImVec2(0.0f, 20.0f));
        ImGui::BulletText("Bullet text demo");
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        raw_hardware_input(window);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();

	return 0;
}