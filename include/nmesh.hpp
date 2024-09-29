#ifndef NMESH_HPP
#define NMESH_HPP

#include<GL/glew.h>
#include<iostream>
#include<string>
#include<fstream>
#include<vector>
#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"

class nmeshvf
{
public:
    unsigned int vao, vbo, ebo; //Vertex array, vertex buffer and element buffer objects.
    std::vector<float> main_buffer; //Final form of the geometry data to draw.
    unsigned int inds_size;

    nmeshvf(const char *objpath)
    {
        std::ifstream fp;
        fp.open(objpath);
        if (!fp.is_open())
        {
            printf("'%s' not found. Exiting...\n", objpath);
            exit(EXIT_FAILURE);
        }

        //Vertices (format : v x y z).
        std::vector<std::vector<float>> verts;
        float x,y,z;

        //Indices (format : f i1 i2 i3).
        std::vector<unsigned int> inds;
        unsigned int i1,i2,i3;

        std::string line;
        while (getline(fp, line))
        {
            if (line[0] == 'v' && line[1] == ' ') //Then we have a vertex line.
            {
                const char *tmp_line = line.c_str();
                sscanf(tmp_line, "v %f %f %f", &x,&y,&z);
                verts.push_back({x,y,z});
            }
            else if (line[0] == 'f') //Then we have an index line.
            {
                const char *tmp_line = line.c_str();
                sscanf(tmp_line, "f %d %d %d", &i1,&i2,&i3);
                inds.push_back(i1-1);
                inds.push_back(i2-1);
                inds.push_back(i3-1);
                //Subtracting 1 to convert to 0-based indexing.
            }
        }

        //Flatten the vertex data into main_buffer[].
        for (const auto &v : verts)
        {
            main_buffer.push_back(v[0]);
            main_buffer.push_back(v[1]);
            main_buffer.push_back(v[2]);
        }

        //Now set up vao, vbo, and ebo.
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, main_buffer.size()*sizeof(float), &main_buffer[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size()*sizeof(unsigned int), &inds[0], GL_STATIC_DRAW);
        glBindVertexArray(0); //Unbind the vao.

        inds_size = inds.size();
    }

    //Delete the mesh.
    ~nmeshvf()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
    }

    //Draw the mesh (triangles).
    void draw_triangles()
    {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, inds_size, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};

#endif
