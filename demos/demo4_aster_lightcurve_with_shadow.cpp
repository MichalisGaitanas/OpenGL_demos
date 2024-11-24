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

int win_width = 500, win_height = 500;

const int shadow_tex_reso_x = 2048, shadow_tex_reso_y = 2048; //Shadow image resolution.

unsigned int fbo_depth, tex_depth; //IDs to hold the depth fbo and the depth texture (shadow map).
unsigned int fbo_lightcurve, rbo_lightcurve, tex_lightcurve; //IDs to hold the fbo, renderbuffer, and texture of the lightcurve.

camera cam(glm::vec3(0.0f, -5.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), 90.0f);

void setup_fbo_depth()
{
    glGenFramebuffers(1, &fbo_depth); //Create fbo and assign ID.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_depth); //This means that all subsequent fb operations affect the fbo_depth.
    glGenTextures(1, &tex_depth); //Create tex and assign ID.
    glBindTexture(GL_TEXTURE_2D, tex_depth); //This means that all subsequent tex operations affect the tex_depth.
    //Actually create the depth texture with the specified resolution. Stored as floats and initialized as NULL because no data is provided yet.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, shadow_tex_reso_x, shadow_tex_reso_y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //The following ensures that when texture coordinates go outside [0,1] range, the border_col is used as depth-color.
    float border_col[] = {1.0f, 1.0f, 1.0f, 1.0f}; //Pure white that is, coz white color corresponds to maximum depth.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_col);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_depth, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr, "Shadow framebuffer is not completed!\n");

    //Since shadow mapping only requires depth information and needs no colors, the following commnads make sure that
    //OpenGL avoids any (unnecessary) color buffer operations.
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setup_fbo_lightcurve(int width_pix, int height_pix)
{
    //If memory resources are already allocated, delete them first.
    if (fbo_lightcurve)
    {
        glDeleteFramebuffers(1, &fbo_lightcurve);
        glDeleteTextures(1, &tex_lightcurve);
        glDeleteRenderbuffers(1, &rbo_lightcurve);
    }

    //Create a framebuffer object (fbo). This is basically similar as the process of creating vbo, vao, ebo, etc...
    glGenFramebuffers(1, &fbo_lightcurve);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_lightcurve);

    //Create a texture (tex) to render to.
    glGenTextures(1, &tex_lightcurve);
    glBindTexture(GL_TEXTURE_2D, tex_lightcurve);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width_pix, height_pix, 0, GL_RED, GL_FLOAT, NULL); //Grayscale values only (red channel only that is).
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //Attach the texture to the hidden framebuffer.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_lightcurve, 0);
    
    //Create a renderbuffer for depth and stencil.
    glGenRenderbuffers(1, &rbo_lightcurve);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_lightcurve);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width_pix, height_pix);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_lightcurve);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr, "Lightcurve framebuffer is not completed!\n");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

float get_brightness(unsigned int tex, int width_pix, int height_pix)
{
    glBindTexture(GL_TEXTURE_2D, tex);
    std::vector<float> pixels(width_pix*height_pix);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, pixels.data()); //Read the pixels from the texture (only the red channel, i.e. grayscale color).
    float brightness = 0.0f;
    for (int i = 0; i < width_pix*height_pix; ++i)
        brightness += pixels[i];
    return brightness/(width_pix*height_pix); //Normalize the brightness.
}

//For discrete keyboard events.
void key_callback(GLFWwindow *window, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(window, true);
}

//When the framebuffer resizes, do the following :
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
    //Setup glfw.
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "Real time shadow", NULL, NULL);
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

    glfwGetWindowSize(window, &win_width, &win_height);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }

    //Setup gui stuff. 
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL; //Fucking .ini file!
    io.Fonts->AddFontFromFileTTF("../fonts/Arial.ttf", 15.0f);
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGuiStyle &imstyle = ImGui::GetStyle();
    imstyle.WindowMinSize = ImVec2(200.0f,200.0f);
    imstyle.FrameRounding = 5.0f;
    imstyle.WindowRounding = 5.0f;

    meshvfn asteroid("../obj/vfn/asteroids/gerasimenko256k.obj");
    float fc = 1.1f, fl = 1.2;
    float rc = asteroid.get_farthest_vertex_distance();
    float ang_vel_z = glm::pi<float>()/20.0f;

    //Shaders : 1 for the scene as perceived by the directional light and 1 for the scene as perceived by the camera. The first shader is gonna
    //be used to calculate a special info only (depth). The second shader is gonna use that info to compute all the fragment colors (ambient, diffuse, etc... AND shadows).
    shader shad_depth("../shaders/vertex/trans_dir_light_mvp.vert","../shaders/fragment/nothing.frag");
    shader shad_dir_light_with_shadow("../shaders/vertex/trans_mvpn_shadow.vert","../shaders/fragment/dir_light_d_shadow.frag");

    setup_fbo_depth();
    setup_fbo_lightcurve(win_width, win_height);

    //Constant mesh and light colors. We pass them to the shader from now to avoid doing it in the while loop...
    glm::vec3 mesh_col = glm::vec3(1.0f,1.0f,1.0f);
    glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f);
    shad_dir_light_with_shadow.use();
    shad_dir_light_with_shadow.set_vec3_uniform("mesh_col", mesh_col);
    shad_dir_light_with_shadow.set_vec3_uniform("light_col", light_col);

    glm::mat4 dir_light_projection, dir_light_view, dir_light_pv; //Directional light's matrices.

    glm::mat4 projection, view, model; //Camera's matrices. The 'model' matrix is common.

    //Actual lightcure data.
    std::vector<float> time_vector;
    std::vector<float> brightness_vector;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    float t0 = 0.0f, tnow;
    while (!glfwWindowShouldClose(window))
    {   

        static float dir_light_lon = 80.0f, dir_light_lat = 50.0f;
        glm::vec3 light_dir = fl*rc*glm::vec3(cos(glm::radians(dir_light_lon))*sin(glm::radians(dir_light_lat)),
                                              sin(glm::radians(dir_light_lon))*sin(glm::radians(dir_light_lat)),
                                              cos(glm::radians(dir_light_lat)));
        glm::vec3 norm_light_dir = glm::normalize(light_dir);
        float dir_light_up_x = 0.0f, dir_light_up_y = 0.0f, dir_light_up_z = 1.0f;
        if (glm::abs(norm_light_dir.z) > 0.999f)
        {
            dir_light_up_y = 1.0f;
            dir_light_up_z = 0.0f;
        }

        dir_light_projection = glm::ortho(-fc*rc,fc*rc, -fc*rc,fc*rc, (fl-fc)*rc, 2.0f*fc*rc);
        dir_light_view = glm::lookAt(light_dir, glm::vec3(0.0f), glm::vec3(dir_light_up_x, dir_light_up_y, dir_light_up_z));
        dir_light_pv = dir_light_projection*dir_light_view; //Directional light's projection*view (total) matrix.

        //Camera's updated parameters.
        projection = glm::infinitePerspective(glm::radians(cam.fov), (float)win_width/win_height, 0.05f);
        view = cam.view();

        //Bind the fbo_depth to render the shadow map.
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_depth);
        glViewport(0,0, shadow_tex_reso_x,shadow_tex_reso_y);
        glClear(GL_DEPTH_BUFFER_BIT); //Clear only depth, coz we write only depth in this buffer. There's no color attachment.
        shad_depth.use();
        shad_depth.set_mat4_uniform("dir_light_pv", dir_light_pv);
        //Now transform the models and render to the fbo_depth.
        model = glm::rotate(glm::mat4(1.0f), ang_vel_z*(float)glfwGetTime(), glm::vec3(0.0f,0.0f,1.0f));
        shad_depth.set_mat4_uniform("model", model);
        asteroid.draw_triangles();

        // Render to lightcurve framebuffer.
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_lightcurve);
        glViewport(0, 0, win_width, win_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shad_dir_light_with_shadow.use();
        shad_dir_light_with_shadow.set_mat4_uniform("projection", dir_light_projection);
        shad_dir_light_with_shadow.set_mat4_uniform("view", dir_light_view);
        shad_dir_light_with_shadow.set_vec3_uniform("light_dir", light_dir);
        shad_dir_light_with_shadow.set_mat4_uniform("dir_light_pv", dir_light_pv);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_depth);
        shad_dir_light_with_shadow.set_int_uniform("sample_shadow", 0);
        shad_dir_light_with_shadow.set_mat4_uniform("model", model);
        asteroid.draw_triangles();
        brightness_vector.push_back(get_brightness(tex_lightcurve, win_width, win_height));
        time_vector.push_back(t);
        t += dt;

        //Bind the default fbo to render the scene to the window.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0,0, win_width, win_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Now we have both depth and color (unlike to the fbo_depth).
        asteroid.draw_triangles();        

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(300.0f, 600.0f), ImGuiCond_FirstUseEver);
        static bool popen = true;
        ImGui::Begin("Controls", &popen); //Imgui window with title and a close button.
        if (!popen)
            glfwSetWindowShouldClose(window, true);

        ImGui::BulletText("Light's direction");
        ImGui::SliderFloat("lon [deg]##dir_light_lon", &dir_light_lon, 0.0f, 360.0f);
        ImGui::SliderFloat("lat [deg]##dir_light_lat", &dir_light_lat, 0.0f, 180.0f);

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
       
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    FILE *fp_lightcurve = fopen("demo4_lightcurve.txt","w");
    for (size_t i = 0; i < time_vector.size(); ++i)
        fprintf(fp_lightcurve,"%.6f  %.6f\n",time_vector[i], brightness_vector[i]);
    fclose(fp_lightcurve);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}
