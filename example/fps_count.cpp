#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<iostream>
#include<string>
#include"../headers/shader.h"
#include"../headers/mesh.h"
#include"../headers/camera.h"
#include"../headers/font.h"

using namespace std;

int winWidth, winHeight;
float aspectRatio;

double tglfw, tprev, msPerFrame;
int frames = 0;

//objs to draw
int drawDidymain = 1; string didymainOnOff;
int drawBennu = 1; string bennuOnOff;
int drawItokawa = 1; string itokawaOnOff;
int drawChuryumov = 1; string churyumovOnOff;
int drawEros = 1; string erosOnOff;
int drawKleopatra = 1; string kleopatraOnOff;
int drawVesta = 1; string vestaOnOff;

camera cam; //default camera

void key_callback(GLFWwindow *win, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(win, true);
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
        drawDidymain = !drawDidymain;
    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
        drawBennu = !drawBennu;
    if (key == GLFW_KEY_3 && action == GLFW_PRESS)
        drawItokawa = !drawItokawa;
    if (key == GLFW_KEY_4 && action == GLFW_PRESS)
        drawChuryumov = !drawChuryumov;
    if (key == GLFW_KEY_5 && action == GLFW_PRESS)
        drawEros = !drawEros;
    if (key == GLFW_KEY_6 && action == GLFW_PRESS)
        drawKleopatra = !drawKleopatra;
    if (key == GLFW_KEY_7 && action == GLFW_PRESS)
        drawVesta = !drawVesta;
}

void framebuffer_size_callback(GLFWwindow *win, int w, int h)
{
    if (w == 0) w = 1;
    if (h == 0) h = 1;
    glViewport(0,0,w,h);
    winWidth = w;
    winHeight = h;
    aspectRatio = (float)w/h;
}

GLFWwindow *window()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow *win = glfwCreateWindow(800, 600, "65803 Didymos", NULL, NULL);
    if (win == NULL)
    {
        cout << "Failed to create glfw window. Exiting...\n";
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(win);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize glew. Exiting...\n";
        glfwTerminate();
        return 0;
    }
    glfwSetWindowSizeLimits(win, 400, 400, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
    glfwSetKeyCallback(win, key_callback);
    glfwGetWindowSize(win, &winWidth, &winHeight);
    aspectRatio = (float)winWidth/winHeight;

    return win;
}

int main()
{
    GLFWwindow *win = window();

    mesh didymain("../models/flat/didymain.obj",1,1,1);
    mesh bennu("../models/flat/bennu.obj",1,1,1);
    mesh itokawa("../models/flat/itokawa.obj",1,1,1);
    mesh churyumov("../models/flat/churyumov.obj",1,1,1);
    mesh eros("../models/flat/eros.obj",1,1,1);
    mesh kleopatra("../models/flat/kleopatra.obj",1,1,1);
    mesh vesta("../models/flat/vesta.obj",1,1,1);

    font ttf("../fonts/NotoSansRegular.ttf");

    shader mvpn_plight_ad("../shaders/trans_mvpn.vert","../shaders/point_light_ad.frag");
    shader textShad("../shaders/text.vert","../shaders/text.frag");

    //constants like
    glm::vec3 lightPos = glm::vec3(0.0f,-100.0f,0.0f);
    glm::vec3 lightCol = glm::vec3(1.0f,1.0f,1.0f);
    glm::vec3 asterCol = glm::vec3(0.5f,0.5f,0.5f);

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
            msPerFrame = 1000.0/(double)frames;
            frames = 0;
            tprev += 1.0;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.01f, 5000.0f);
        view = cam.view();
        model = glm::mat4(1.0f);

        mvpn_plight_ad.use();
        mvpn_plight_ad.set_mat4_uniform("projection", projection);
        mvpn_plight_ad.set_mat4_uniform("view", view);
        mvpn_plight_ad.set_mat4_uniform("model", model);
        mvpn_plight_ad.set_vec3_uniform("lightPos", lightPos);
        mvpn_plight_ad.set_vec3_uniform("lightCol", lightCol);
        mvpn_plight_ad.set_vec3_uniform("modelCol", asterCol);

        //didymain
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0f,3.0f,0.0));
        model = glm::rotate(model, (float)tglfw/10.0f, glm::vec3(0.0f,0.0f,1.0f));
        mvpn_plight_ad.use();
        mvpn_plight_ad.set_mat4_uniform("model", model);
        didymainOnOff = "off";
        if (drawDidymain)
        {
            didymain.draw_triangles();
            didymainOnOff = "on";
        }

        //bennu
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f,3.0f,0.0));
        model = glm::rotate(model, (float)tglfw/10.0f, glm::vec3(0.0f,0.0f,1.0f));
        mvpn_plight_ad.use();
        mvpn_plight_ad.set_mat4_uniform("model", model);
        bennuOnOff = "off";
        if (drawBennu)
        {
            bennu.draw_triangles();
            bennuOnOff = "on";
        }

        //itokawa
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.75f,3.0f,0.0));
        model = glm::rotate(model, (float)tglfw/10.0f, glm::vec3(0.0f,0.0f,1.0f));
        mvpn_plight_ad.use();
        mvpn_plight_ad.set_mat4_uniform("model", model);
        itokawaOnOff = "off";
        if (drawItokawa)
        {
            itokawa.draw_triangles();
            itokawaOnOff = "on";
        }

        //churyumov
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.0f,10.0f,-1.0));
        model = glm::rotate(model, (float)tglfw/10.0f, glm::vec3(0.0f,0.0f,1.0f));
        mvpn_plight_ad.use();
        mvpn_plight_ad.set_mat4_uniform("model", model);
        churyumovOnOff = "off";
        if (drawChuryumov)
        {
            churyumov.draw_triangles();
            churyumovOnOff = "on";
        }

        //eros
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(30.0f,100.0f,20.0));
        model = glm::rotate(model, (float)tglfw/10.0f, glm::vec3(0.0f,0.0f,1.0f));
        mvpn_plight_ad.use();
        mvpn_plight_ad.set_mat4_uniform("model", model);
        erosOnOff = "off";
        if (drawEros)
        {
            eros.draw_triangles();
            erosOnOff = "on";
        }

        //kleopatra
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-100.0f,300.0f,20.0));
        model = glm::rotate(model, (float)tglfw/10.0f, glm::vec3(0.0f,0.0f,1.0f));
        mvpn_plight_ad.use();
        mvpn_plight_ad.set_mat4_uniform("model", model);
        kleopatraOnOff = "off";
        if (drawKleopatra)
        {
            kleopatra.draw_triangles();
            kleopatraOnOff = "on";
        }

        //vesta
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f,700.0f,0.0));
        model = glm::rotate(model, (float)tglfw/10.0f, glm::vec3(0.0f,0.0f,1.0f));
        mvpn_plight_ad.use();
        mvpn_plight_ad.set_mat4_uniform("model", model);
        vestaOnOff = "off";
        if (drawVesta)
        {
            vesta.draw_triangles();
            vestaOnOff = "on";
        }

        //itokawa
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.75f,3.0f,0.0));
        model = glm::rotate(model, (float)tglfw/10.0f, glm::vec3(0.0f,0.0f,1.0f));
        mvpn_plight_ad.use();
        mvpn_plight_ad.set_mat4_uniform("model", model);
        itokawaOnOff = "off";
        if (drawItokawa)
        {
            itokawa.draw_triangles();
            itokawaOnOff = "on";
        }

        //text
        char text[100];
        sprintf(text, "Time : %d [sec]", (int)tglfw);
        ttf.draw(text, 20.0f, winHeight - 50.0f, winWidth, winHeight, 0.3f, glm::vec3(0.0f,0.75f,0.0f), textShad);
        sprintf(text, "------------------------");
        ttf.draw(text, 20.0f, winHeight - 65.0f, winWidth, winHeight, 0.3f, glm::vec3(0.0f,0.75f,0.0f), textShad);
        sprintf(text, "Didymain :  1  (%s)", didymainOnOff.c_str());
        ttf.draw(text, 20.0f, winHeight - 80.0f, winWidth, winHeight, 0.3f, glm::vec3(0.0f,0.75f,0.0f), textShad);
        sprintf(text, "Bennu :  2  (%s)", bennuOnOff.c_str());
        ttf.draw(text, 20.0f, winHeight - 100.0f, winWidth, winHeight, 0.3f, glm::vec3(0.0f,0.75f,0.0f), textShad);
        sprintf(text, "Itokawa :  3  (%s)", itokawaOnOff.c_str());
        ttf.draw(text, 20.0f, winHeight - 120.0f, winWidth, winHeight, 0.3f, glm::vec3(0.0f,0.75f,0.0f), textShad);
        sprintf(text, "Churyumov :  4  (%s)", churyumovOnOff.c_str());
        ttf.draw(text, 20.0f, winHeight - 140.0f, winWidth, winHeight, 0.3f, glm::vec3(0.0f,0.75f,0.0f), textShad);
        sprintf(text, "Eros : 5  (%s)", erosOnOff.c_str());
        ttf.draw(text, 20.0f, winHeight - 160.0f, winWidth, winHeight, 0.3f, glm::vec3(0.0f,0.75f,0.0f), textShad);
        sprintf(text, "Kleopatra : 6  (%s)", kleopatraOnOff.c_str());
        ttf.draw(text, 20.0f, winHeight - 180.0f, winWidth, winHeight, 0.3f, glm::vec3(0.0f,0.75f,0.0f), textShad);
        sprintf(text, "Vesta :  7  (%s)", vestaOnOff.c_str());
        ttf.draw(text, 20.0f, winHeight - 200.0f, winWidth, winHeight, 0.3f, glm::vec3(0.0f,0.75f,0.0f), textShad);

        sprintf(text, "ms/frame : %f", (float)msPerFrame);
        ttf.draw(text, 20.0f, winHeight - 240.0f, winWidth, winHeight, 0.3f, glm::vec3(0.75f,0.0f,0.0f), textShad);
        sprintf(text, "avg fps : %d", (int)(1000.0/msPerFrame));
        ttf.draw(text, 20.0f, winHeight - 260.0f, winWidth, winHeight, 0.3f, glm::vec3(0.75f,0.0f,0.0f), textShad);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
