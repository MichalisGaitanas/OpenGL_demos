#ifndef FONT_HPP
#define FONT_HPP

#include<GL/glew.h>
#include<glm/glm.hpp>
#include<iostream>
#include<string>
#include<map>
#include<cmath>
#include<cstdlib>
#include<ft2build.h>
#include FT_FREETYPE_H

using namespace std;

class character
{
public:
    unsigned int tex_id; //ID handle of the glyph texture
    glm::ivec2 glyph_size; //size of glyph
    glm::ivec2 bearing; //offset from baseline to left/top of glyph
    long int advance; //horizontal offset to advance to next glyph
};

class font
{
public:
    map<GLchar, character> characters;
    unsigned int vao,vbo;
    glm::mat4 projection; //orthographic projection matrix for the text

    font(const char *path)
    {
        //initialize FreeType
        FT_Library ft;
        if (FT_Init_FreeType(&ft))
        {
            printf("Error while initializing FreeType library. Exiting...\n");
            exit(EXIT_FAILURE);
        }
        //load font as face
        FT_Face face;
        if (FT_New_Face(ft, path, 0, &face))
        {
            printf("Font not found. Exiting...\n");
            exit(EXIT_FAILURE);
        }
        FT_Set_Pixel_Sizes(face, 0, 48); //set size to load glyphs as
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //disable byte-alignment restriction
        //load first 128 characters of ASCII set
        for (GLubyte c = 0; c < 128; c++)
        {
            //load character glyph
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                printf("Failed to load current glyph. Proceeding to the next\n");
                continue;
            }
            //generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         GL_RED,
                         face->glyph->bitmap.width,
                         face->glyph->bitmap.rows,
                         0,
                         GL_RED,
                         GL_UNSIGNED_BYTE,
                         face->glyph->bitmap.buffer);
            //set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            //now store character for later use
            character chr = { texture,
                              glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                              glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                              face->glyph->advance.x };
            characters.insert(pair<GLchar, character>(c, chr));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        //destroy FreeType once we're finished
        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        //configure vao,vbo for texture quads
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*6*4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    ~font()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

    void draw(string text, float x, float y, int win_width, int win_height, float scale, glm::vec3 color, shader &shad)
    {
        //set the text's coordinate system
        projection = glm::ortho(0.0f, (float)win_width, 0.0f, (float)win_height);
        shad.use(); //activate the text shader
        shad.set_mat4_uniform("projection", projection);
        shad.set_vec3_uniform("text_col", color);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(vao);

        //iterate through all characters
        string::const_iterator c;
        for (c = text.begin(); c != text.end(); c++)
        {
            character ch = characters[*c];
            float xpos = x + ch.bearing.x*scale;
            float ypos = y - (ch.glyph_size.y - ch.bearing.y)*scale;
            float w = ch.glyph_size.x*scale;
            float h = ch.glyph_size.y*scale;
            //update vbo for each character
            float verts[6][4] =
            {
                { xpos,     ypos + h,   0.0, 0.0 },
                { xpos,     ypos,       0.0, 1.0 },
                { xpos + w, ypos,       1.0, 1.0 },

                { xpos,     ypos + h,   0.0, 0.0 },
                { xpos + w, ypos,       1.0, 1.0 },
                { xpos + w, ypos + h,   1.0, 0.0 }
            };

            glBindTexture(GL_TEXTURE_2D, ch.tex_id); //render glyph texture over quad
            //update content of vbo memory
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glDrawArrays(GL_TRIANGLES, 0, 6); //actual render function
            //now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += (ch.advance >> 6)*scale; //bitshift by 6 to get value in pixels
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};
#endif
