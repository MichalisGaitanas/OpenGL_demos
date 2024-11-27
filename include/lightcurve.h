#ifndef LIGHTCURVE_H
#define LIGHTCURVE_H

#include<GL/glew.h>
#include<vector>
#include<numeric>

#include"shader.h"

float get_brightness_cpu(unsigned int texture, int width_pix, int height_pix)
{
    std::vector<float> pixels(width_pix*height_pix);
    float brightness = 0.0f;
    glBindTexture(GL_TEXTURE_2D, texture);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, pixels.data()); //Read the pixels RED (grayscale) values from the bound lightcurve texture.
    for (int i = 0; i < width_pix*height_pix; ++i)
        brightness += pixels[i];
    glBindTexture(GL_TEXTURE_2D, 0);
    return brightness/(width_pix*height_pix);
}

float get_brightness_gpu(unsigned int texture, int width_pix, int height_pix)
{
    static bool initialized = false;
    static cshader shadb("../shaders/compute/brightness.comp");
    static unsigned int ssbo_sums;
    static const unsigned int local_size_x = 16, local_size_y = 16;
    static unsigned int num_groups_x, num_groups_y;
    static int prev_width = 0, prev_height = 0;

    if (!initialized || width_pix != prev_width || height_pix != prev_height)
    {
        num_groups_x = (width_pix + local_size_x - 1)/local_size_x;
        num_groups_y = (height_pix + local_size_y - 1)/local_size_y;

        unsigned int ssbo_size = num_groups_x*num_groups_y*sizeof(float);

        if (initialized)
            glDeleteBuffers(1, &ssbo_sums);

        glGenBuffers(1, &ssbo_sums);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_sums);
        glBufferData(GL_SHADER_STORAGE_BUFFER, ssbo_size, NULL, GL_DYNAMIC_COPY);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_sums); //Binding point 1.
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        initialized = true;
        prev_width = width_pix;
        prev_height = height_pix;
    }

    shadb.use();
    shadb.set_ivec2_uniform("image_size", width_pix, height_pix);
    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
    shadb.dispatch(num_groups_x, num_groups_y, 1);
    //Read back the partial sums :
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_sums);
    float *partial_sums = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

    float brightness = 0.0f;
    int num_partial_sums = num_groups_x*num_groups_y;
    for (int i = 0; i < num_partial_sums; ++i)
        brightness += partial_sums[i];

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    return brightness/(width_pix*height_pix);
}

#endif