#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include<cstdio>
#include<cmath>

#include"../include/shader.hpp"
#include"../include/mesh.hpp"

int win_width = 1500, win_height = 900;
unsigned int fbo, fbo_tex, rbo; //Framebuffer object, framebuffer object (attached) textured and renderbuffer object.

void setup_framebuffer(int width, int height)
{   
    //If it exists, delete it first. This if statement will NOT run in the first fbo setup, coz the fbo will not exist,
    //but WILL run every time the framebuffer is resized.
    if (fbo != 0)
    {
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &fbo_tex);
        glDeleteRenderbuffers(1, &rbo);
    }

    //Generate a new framebuffer.
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    //Generate a new texture to store the rendered scene.
    glGenTextures(1, &fbo_tex);
    glBindTexture(GL_TEXTURE_2D, fbo_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //Attach the generated teture the framebuffer.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_tex, 0);

    //Now generate the depth buffer. Remember, we have to do this manually, unlike the default depth buffer, which
    //is generated either by glfw or the OpenGL kernel. A framebuffer is basically a container that holds multiple
    //attachments (textures or renderbuffers) for storing the results of rendering operations. The framebuffer itself
    //doesn’t directly store data. Instead, it holds references to other objects, like textures or renderbuffers, which
    //are used to store color, depth, and stencil information. Framebuffers seem to work similarly with vaos and vbos.
    //As I was saying..., You need a bunch of buffers to work. For the default framebuffer, everything is setup for you.
    //However when you create a new framebuffer, you also have to setup the addtitional-for-rendering buffers (depth buffer in our case).
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
    //Optional for both depth AND stencil buffer.
    //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Framebuffer is not completed. Exiting...\n");
        exit(EXIT_FAILURE);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0); //Unbind the fbo.
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow *win, int w, int h)
{
    win_width = w;
    win_height = h;
    glViewport(0,0,w,h);
    setup_framebuffer(win_width, win_height);
}

void glfw_center_window(GLFWwindow *win)
{
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwGetWindowSize(win, &win_width, &win_height);
    int centx = (mode->width - win_width)/2;
    int centy = (mode->height - win_height)/2;
    glfwSetWindowPos(win, centx, centy);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "Blurry scene", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    glfw_center_window(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }

    //Load the meshes with the corresponding textures.
    meshvft ground("../obj/vft/plane10x10.obj", "../images/texture/aerial_grass_rock_diff_4k.jpg");
    meshvft wooden_stool("../obj/vft/wooden_stool.obj", "../images/texture/wooden_stool_diff_2k.jpg");
    meshvft brick_cube("../obj/vft/cube1x1x1_correct_uv.obj", "../images/texture/red_brick_diff_2k.jpg");
    meshvft wooden_container("../obj/vft/cube1x1x1_correct_uv.obj", "../images/texture/wooden_container_diff_512x512.jpg");
    meshvft plant_pot("../obj/vft/plant_pot.obj", "../images/texture/potted_plant_pot_diff_2k.png");
    meshvft plant_leaves("../obj/vft/plant_leaves.obj", "../images/texture/potted_plant_leaves_diff_2k.png");
    shader texshad("../shaders/vertex/trans_mvp_texture.vert","../shaders/fragment/texture.frag");

    quadtex quad;
    shader blurshad("../shaders/vertex/trans_nothing_texture.vert", "../shaders/fragment/blur.frag");
    setup_framebuffer(win_width, win_height);

    glm::mat4 projection, view, model;

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f,0.7f,1.0f,1.0f);
    while (!glfwWindowShouldClose(window))
    {
        /* First rendering pass : Render the entire 3D scene in the fbo, which we will never see it in the monitor. */

        texshad.use();
        glBindFramebuffer(GL_FRAMEBUFFER, fbo); //Bind the "hidden" framebuffer.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Apply clearance commands (to the "hidden" framebuffer).

        projection = glm::perspective(glm::radians(45.0f), (float)win_width/(float)win_height, 0.01f,100.0f);
        view = glm::lookAt(glm::vec3(5.0f*(float)cos(0.1f*glfwGetTime()),5.0f*(float)sin(0.1f*glfwGetTime()),2.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,2.0f));
        texshad.set_mat4_uniform("projection", projection);
        texshad.set_mat4_uniform("view", view);

        //Ground :
        model = glm::mat4(1.0f);
        texshad.set_mat4_uniform("model", model);
        ground.draw_triangles();

        //Wooden stool :
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f,0.0f,0.0f));
        texshad.set_mat4_uniform("model", model);
        wooden_stool.draw_triangles();

        //Brick cube :
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0f,0.5f,0.5f));
        texshad.set_mat4_uniform("model", model);
        brick_cube.draw_triangles();

        //Wooden container :
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f,-0.8f,0.5f));
        texshad.set_mat4_uniform("model", model);
        wooden_container.draw_triangles();

        //Plant (pot and leaves) :
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.7f,0.7f,0.0f)); //Redundant...
        texshad.set_mat4_uniform("model", model);
        plant_pot.draw_triangles();
        plant_leaves.draw_triangles();

        /* Second rendering pass : Render only 1 windowed-fullscreen quad in the displayed fbo. The whole 3D scene however is
           visible in the second rendering pass because the quad’s texture is the rendered scene itself. The quad essentially acts as a
           canvas for displaying the pre-rendered scene. Since the quad covers the entire screen and the texture it uses
           is the complete scene, it looks like we are rendering the scene again, but really we are just displaying the
           result of the first pass. We do this to apply apply the blur effect. But why not directly in the "texture.frag" shader? Why
           do we write another shader? Well, because the first fragment shader only deals with local information related to the mesh.
           For a blur effect (or other effects), we need to operate on scene as whole image, because we have to take into account
           neighboring pixels everywhere in the renered scene. The classical fragment shader of a mesh doesn't have access to
           information about other pixels outside the mesh that is drawing. Even though one could perhaps modify the fragment shader of
           a mesh in a way that a blur (or other) effect is achieved, it is not recommended. What if you have 4 different fragment shaders
           in your rendered scene? Would you alter all 4 shaders (much code, poor performance)? What about the seperating scene regions
           where one mesh gets in front of another (those regions will not be blurred)? However operating on the scene as an image-globally,
           yields global scene effects, which is the desired. Enough for a code comment...
         */

        blurshad.use();
        glBindFramebuffer(GL_FRAMEBUFFER, 0); //Bind to the default framebuffer (the one we will see in the monitor).
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Apply clearance commands (to the displayed framebuffer).
        quad.draw_triangles(fbo_tex); //Draw only the quad.

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
