#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<cstdio>
#include<string>
#include"../include/shader.hpp"
#include"../include/mesh.hpp"
#include"../include/camera.hpp"
#include"../include/font.hpp"

int win_width, win_height;
float aspect_ratio;

double tglfw, tprev, ms_per_frame;
int frames = 0;

//objs to draw
int draw_didymain = 1; std::string didymain_onoff;
int draw_bennu = 1; std::string bennu_onoff;
int draw_itokawa = 1; std::string itokawa_onoff;
int draw_churyumov = 1; std::string churyumov_onoff;
int draw_eros = 1; std::string eros_οnοff;
int draw_kleopatra = 1; std::string kleopatra_onoff;

camera cam; //default camera constructor

void key_callback(GLFWwindow *win, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(win, true);
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
        draw_didymain = !draw_didymain;
    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
        draw_bennu = !draw_bennu;
    if (key == GLFW_KEY_3 && action == GLFW_PRESS)
        draw_itokawa = !draw_itokawa;
    if (key == GLFW_KEY_4 && action == GLFW_PRESS)
        draw_churyumov = !draw_churyumov;
    if (key == GLFW_KEY_5 && action == GLFW_PRESS)
        draw_eros = !draw_eros;
    if (key == GLFW_KEY_6 && action == GLFW_PRESS)
        draw_kleopatra = !draw_kleopatra;
}

void framebuffer_size_callback(GLFWwindow *win, int w, int h)
{
    if (w == 0) w = 1;
    if (h == 0) h = 1;
    glViewport(0,0,w,h);
    win_width = w;
    win_height = h;
    aspect_ratio = (float)w/h;
}

GLFWwindow *window()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow *win = glfwCreateWindow(800, 600, "Asteroids", NULL, NULL);
    if (win == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(win);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwSetWindowSizeLimits(win, 400, 400, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
    glfwSetKeyCallback(win, key_callback);
    glfwGetWindowSize(win, &win_width, &win_height);
    aspect_ratio = (float)win_width/win_height;

    return win;
}

int main()
{
    GLFWwindow *win = window();

    mesh didymain("../obj/vert_face_fnorm//didymos_binary/didymain2019.obj",1,1,1);
    mesh bennu("../obj/vert_face_fnorm/bennu.obj",1,1,1);
    mesh itokawa("../obj/vert_face_fnorm/itokawa.obj",1,1,1);
    mesh churyumov("../obj/vert_face_fnorm/churyumov.obj",1,1,1);
    mesh eros("../obj/vert_face_fnorm/eros.obj",1,1,1);
    mesh kleopatra("../obj/vert_face_fnorm/kleopatra.obj",1,1,1);

    font ttf("../font/NotoSansRegular.ttf");

    shader mvpn_plight_ad("../shader/vertex/trans_mvpn.vert","../shader/fragment/point_light_ad.frag");
    shader text_shad("../shader/vertex/trans_nothing_text.vert","../shader/fragment/text.frag");

    glm::vec3 light_pos = glm::vec3(0.0f,-100.0f,0.0f);
    glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f);
    glm::vec3 aster_col = glm::vec3(0.5f,0.5f,0.5f);

    glm::mat4 projection, view, model; //glm matrices

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.05f,0.05f,0.05f,1.0f);

    tprev = glfwGetTime();
    while (!glfwWindowShouldClose(win))
    {
        tglfw = glfwGetTime(); //elapsed time in [sec] since glfwInit()
        frames++;
        if (tglfw - tprev >= 1.0)
        {
            ms_per_frame = 1000.0/(double)frames;
            frames = 0;
            tprev += 1.0;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        projection = glm::perspective(glm::radians(45.0f), aspect_ratio, 0.01f, 5000.0f);
        view = cam.view();
        model = glm::mat4(1.0f);

        mvpn_plight_ad.use();
        mvpn_plight_ad.set_mat4_uniform("projection", projection);
        mvpn_plight_ad.set_mat4_uniform("view", view);
        mvpn_plight_ad.set_mat4_uniform("model", model);
        mvpn_plight_ad.set_vec3_uniform("light_pos", light_pos);
        mvpn_plight_ad.set_vec3_uniform("light_col", light_col);
        mvpn_plight_ad.set_vec3_uniform("model_col", aster_col);

        //didymain
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0f,3.0f,0.0));
        model = glm::rotate(model, (float)tglfw/10.0f, glm::vec3(0.0f,0.0f,1.0f));
        mvpn_plight_ad.use();
        mvpn_plight_ad.set_mat4_uniform("model", model);
        didymain_onoff = "off";
        if (draw_didymain)
        {
            didymain.draw_triangles();
            didymain_onoff = "on";
        }

        //bennu
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f,3.0f,0.0));
        model = glm::rotate(model, (float)tglfw/10.0f, glm::vec3(0.0f,0.0f,1.0f));
        mvpn_plight_ad.use();
        mvpn_plight_ad.set_mat4_uniform("model", model);
        bennu_onoff = "off";
        if (draw_bennu)
        {
            bennu.draw_triangles();
            bennu_onoff = "on";
        }

        //itokawa
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.75f,3.0f,0.0));
        model = glm::rotate(model, (float)tglfw/10.0f, glm::vec3(0.0f,0.0f,1.0f));
        mvpn_plight_ad.use();
        mvpn_plight_ad.set_mat4_uniform("model", model);
        itokawa_onoff = "off";
        if (draw_itokawa)
        {
            itokawa.draw_triangles();
            itokawa_onoff = "on";
        }

        //churyumov
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.0f,10.0f,-1.0));
        model = glm::rotate(model, (float)tglfw/10.0f, glm::vec3(0.0f,0.0f,1.0f));
        mvpn_plight_ad.use();
        mvpn_plight_ad.set_mat4_uniform("model", model);
        churyumov_onoff = "off";
        if (draw_churyumov)
        {
            churyumov.draw_triangles();
            churyumov_onoff = "on";
        }

        //eros
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(30.0f,100.0f,20.0));
        model = glm::rotate(model, (float)tglfw/10.0f, glm::vec3(0.0f,0.0f,1.0f));
        mvpn_plight_ad.use();
        mvpn_plight_ad.set_mat4_uniform("model", model);
        eros_οnοff = "off";
        if (draw_eros)
        {
            eros.draw_triangles();
            eros_οnοff = "on";
        }

        //kleopatra
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-100.0f,300.0f,20.0));
        model = glm::rotate(model, (float)tglfw/10.0f, glm::vec3(0.0f,0.0f,1.0f));
        mvpn_plight_ad.use();
        mvpn_plight_ad.set_mat4_uniform("model", model);
        kleopatra_onoff = "off";
        if (draw_kleopatra)
        {
            kleopatra.draw_triangles();
            kleopatra_onoff = "on";
        }

        //itokawa
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.75f,3.0f,0.0));
        model = glm::rotate(model, (float)tglfw/10.0f, glm::vec3(0.0f,0.0f,1.0f));
        mvpn_plight_ad.use();
        mvpn_plight_ad.set_mat4_uniform("model", model);
        itokawa_onoff = "off";
        if (draw_itokawa)
        {
            itokawa.draw_triangles();
            itokawa_onoff = "on";
        }

        //text
        char text[100];
        sprintf(text, "Time : %d [sec]", (int)tglfw);
        ttf.draw(text, 20.0f, win_height - 50.0f, win_width, win_height, 0.3f, glm::vec3(0.0f,0.75f,0.0f), text_shad);
        sprintf(text, "------------------------");
        ttf.draw(text, 20.0f, win_height - 65.0f, win_width, win_height, 0.3f, glm::vec3(0.0f,0.75f,0.0f), text_shad);
        sprintf(text, "Didymain :  1  (%s)", didymain_onoff.c_str());
        ttf.draw(text, 20.0f, win_height - 80.0f, win_width, win_height, 0.3f, glm::vec3(0.0f,0.75f,0.0f), text_shad);
        sprintf(text, "Bennu :  2  (%s)", bennu_onoff.c_str());
        ttf.draw(text, 20.0f, win_height - 100.0f, win_width, win_height, 0.3f, glm::vec3(0.0f,0.75f,0.0f), text_shad);
        sprintf(text, "Itokawa :  3  (%s)", itokawa_onoff.c_str());
        ttf.draw(text, 20.0f, win_height - 120.0f, win_width, win_height, 0.3f, glm::vec3(0.0f,0.75f,0.0f), text_shad);
        sprintf(text, "Churyumov :  4  (%s)", churyumov_onoff.c_str());
        ttf.draw(text, 20.0f, win_height - 140.0f, win_width, win_height, 0.3f, glm::vec3(0.0f,0.75f,0.0f), text_shad);
        sprintf(text, "Eros : 5  (%s)", eros_οnοff.c_str());
        ttf.draw(text, 20.0f, win_height - 160.0f, win_width, win_height, 0.3f, glm::vec3(0.0f,0.75f,0.0f), text_shad);
        sprintf(text, "Kleopatra : 6  (%s)", kleopatra_onoff.c_str());
        ttf.draw(text, 20.0f, win_height - 180.0f, win_width, win_height, 0.3f, glm::vec3(0.0f,0.75f,0.0f), text_shad);

        sprintf(text, "ms/frame : %f", (float)ms_per_frame);
        ttf.draw(text, 20.0f, win_height - 240.0f, win_width, win_height, 0.3f, glm::vec3(0.75f,0.0f,0.0f), text_shad);
        sprintf(text, "avg fps : %d", (int)(1000.0/ms_per_frame));
        ttf.draw(text, 20.0f, win_height - 260.0f, win_width, win_height, 0.3f, glm::vec3(0.75f,0.0f,0.0f), text_shad);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
