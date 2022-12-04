#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<cstdio>
#include"../include/shader.hpp"
#include"../include/mesh.hpp"
#include"../include/font.hpp"

int win_width, win_height;
float aspect_ratio;

void raw_hardware_input(GLFWwindow *win)
{
    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(win, true);
    }
}

void framebuffer_size_callback(GLFWwindow *win, int w, int h)
{
    //make sure the window will not crash in case of "akward" resize
    if (w == 0) w = 1;
    if (h == 0) h = 1;
    glViewport(0,0,w,h);
    win_width = w;
    win_height = h;
    aspect_ratio = (float)w/h;
}

int main()
{
    //initialize glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //create glfw window object
    win_width = 800;
    win_height = 600;
    aspect_ratio = (float)win_width/win_height;
    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "Aspect ratio", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //function's format : (window, min_width, min_height, max_width, max_height)
    glfwSetWindowSizeLimits(window, 200, 200, GLFW_DONT_CARE, GLFW_DONT_CARE);

    //validate glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }

    //create mesh and corresponding shader
    meshvfn suzanne("../obj/vfn/smooth/suzanne.obj");
    shader suzanne_shader("../shader/vertex/trans_mvpn.vert","../shader/fragment/dir_light_ads.frag");
    //create font and corresponding shader
    font ttf("../font/NotoSansRegular.ttf");
    shader text_shader("../shader/vertex/trans_nothing_text.vert","../shader/fragment/text.frag");

    glm::vec3 light_dir = glm::vec3(1.0f,1.0f,1.0f);
    glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f);
    glm::vec3 suzanne_col = glm::vec3(0.3f,0.4f,0.1f);
    glm::vec3 cam_pos = glm::vec3(8.0f,8.0f,0.0f);
    glm::vec3 cam_aim = glm::vec3(0.0f,0.0f,0.0f);
    glm::vec3 cam_up = glm::vec3(0.0f,0.0f,1.0f);
    glm::mat4 view = glm::lookAt(cam_pos, cam_aim, cam_up); //this is constant because the camera is still
    suzanne_shader.use();
    suzanne_shader.set_vec3_uniform("light_dir", light_dir);
    suzanne_shader.set_vec3_uniform("light_col", light_col);
    suzanne_shader.set_vec3_uniform("model_col", suzanne_col);
    suzanne_shader.set_vec3_uniform("cam_pos", cam_pos);
    suzanne_shader.set_mat4_uniform("view", view);

    glm::mat4 projection; //this changes in case of framebuffer resize, hence we update it in the render loop
    glm::mat4 model; //this also changes in the render loop because the model rotates

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.1f,0.1f,0.1f,1.0f);
    while (!glfwWindowShouldClose(window))
    {
        raw_hardware_input(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //draw suzanne
        projection = glm::perspective(glm::radians(45.0f), aspect_ratio, 0.1f, 100.0f);
        model = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.0f,0.0f,1.0f));
        suzanne_shader.use();
        suzanne_shader.set_mat4_uniform("projection", projection);
        suzanne_shader.set_mat4_uniform("model", model);
        suzanne.draw_triangles();

        //draw text
        char text[50];
        sprintf(text, "t = %.2f [sec]",(float)glfwGetTime());
        ttf.draw(text, 20.0f, 50.0f, win_width, win_height, 0.4f, glm::vec3(1.0f,1.0f,1.0f), text_shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
