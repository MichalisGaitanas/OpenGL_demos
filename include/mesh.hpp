#ifndef MESH_HPP
#define MESH_HPP

#include<GL/glew.h>
#include<iostream>
#include<string>
#include<fstream>
#include<vector>

class mesh
{
public:
    unsigned int vao,vbo; //vertex array and buffer object
    std::vector<float> data; //final form of the geometry data to draw
    unsigned int nverts; //number of vertices to draw

    mesh(const char *path, int load_verts, int load_faces, int load_norms)
    {
        std::ifstream fp;
        fp.open(path);
        if (!fp.is_open())
        {
            printf("'%s' not found. Exiting...\n", path);
            exit(EXIT_FAILURE);
        }

        if (load_verts == 1 && load_faces == 0 && load_norms == 0) //load vertices
        {
            //vertices (format : v x y z)
            std::vector<std::vector<float>> verts;
            float x,y,z;

            std::string line;

            //load the file
            while (getline(fp, line))
            {
                if (line[0] == 'v' && line[1] == ' ') //then we have a vertex line
                {
                    const char *tmp_line = line.c_str();
                    sscanf(tmp_line, "v %f %f %f", &x,&y,&z);
                    verts.push_back({x,y,z});
                }
            }

            //Now construct the main buffer data[] which will have all the data needed for drawing.
            for (int i = 0; i < verts.size(); ++i)
            {
                data.push_back( verts[i][0] );
                data.push_back( verts[i][1] );
                data.push_back( verts[i][2] );
            }
            //data[] has now the form : {x1,y1,z1, x2,y2,z2, x3,y3,z3, ... }, i.e. all the points of the mesh

            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, data.size()*sizeof(float), &data[0], GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            nverts = data.size(); //number of vertices to draw
        }
        else if (load_verts == 1 && load_faces == 1 && load_norms == 0) //load vertices and faces
        {

            //vertices (format : v x y z)
            std::vector<std::vector<float>> verts;
            float x,y,z;

            //indices (format : f i11//i12 i21//i22 i31//i32)
            std::vector<std::vector<unsigned int>> inds;
            unsigned int i11,i12, i21,i22, i31,i32;

            std::string line;

            while (getline(fp, line))
            {
                if (line[0] == 'v' && line[1] == ' ') //then we have a vertex line
                {
                    const char *tmp_line = line.c_str();
                    sscanf(tmp_line, "v %f %f %f", &x,&y,&z);
                    verts.push_back({x,y,z});
                }
                else if (line[0] == 'f') //then we have an indices line
                {
                    const char *tmp_line = line.c_str();
                    sscanf(tmp_line, "f %d//%d %d//%d %d//%d", &i11,&i12, &i21,&i22, &i31,&i32);
                    inds.push_back({i11-1, i21-1, i31-1});
                }
            }

            //Now combine verts[][] and inds[][] to construct the main buffer data[]
            //which will have all the data needed for drawing.
            for (int i = 0; i < inds.size(); ++i)
            {
                data.push_back( verts[ inds[i][0] ][0] );
                data.push_back( verts[ inds[i][0] ][1] );
                data.push_back( verts[ inds[i][0] ][2] );

                data.push_back( verts[ inds[i][1] ][0] );
                data.push_back( verts[ inds[i][1] ][1] );
                data.push_back( verts[ inds[i][1] ][2] );

                data.push_back( verts[ inds[i][2] ][0] );
                data.push_back( verts[ inds[i][2] ][1] );
                data.push_back( verts[ inds[i][2] ][2] );
            }
            //data[] has now the form : {x1,y1,z1, x2,y2,z2, x3,y3,z3, ... }, where
            //consecutive triads of vertices form triangle - faces of the object to be rendered

            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, data.size()*sizeof(float), &data[0], GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            nverts = (int)(data.size()/3); //number of vertices to draw
        }
        else if (load_verts == 1 && load_faces == 1 && load_norms == 1) //load vertices, faces and normals
        {
            //vertices (format : v x y z)
            std::vector<std::vector<float>> verts;
            float x,y,z;

            //norms (format : vn nx ny nz)
            std::vector<std::vector<float>> norms;
            float nx,ny,nz;

            //indices (format : f i11//i12 i21//i22 i31//i32)
            std::vector<std::vector<unsigned int>> inds;
            unsigned int i11,i12, i21,i22, i31,i32;

            std::string line;

            while (getline(fp, line))
            {
                if (line[0] == 'v' && line[1] == ' ') //then we have a vertex line
                {
                    const char *tmp_line = line.c_str();
                    sscanf(tmp_line, "v %f %f %f", &x,&y,&z);
                    verts.push_back({x,y,z});
                }
                else if (line[0] == 'v' && line[1] == 'n') //then we have a normal line
                {
                    const char *tmp_line = line.c_str();
                    sscanf(tmp_line, "vn %f %f %f", &nx,&ny,&nz);
                    norms.push_back({nx,ny,nz});
                }
                else if (line[0] == 'f') //then we have a indices line
                {
                    const char *tmp_line = line.c_str();
                    sscanf(tmp_line, "f %d//%d %d//%d %d//%d", &i11,&i12, &i21,&i22, &i31,&i32);
                    inds.push_back({i11-1, i12-1, i21-1, i22-1, i31-1, i32-1});
                }
            }

            //Now combine verts[][], norms[][] and inds[][] to construct the main buffer data[]
            //which will have all the data needed for drawing.
            for (int i = 0; i < inds.size(); ++i)
            {
                data.push_back( verts[ inds[i][0] ][0] );
                data.push_back( verts[ inds[i][0] ][1] );
                data.push_back( verts[ inds[i][0] ][2] );
                data.push_back( norms[ inds[i][1] ][0] );
                data.push_back( norms[ inds[i][1] ][1] );
                data.push_back( norms[ inds[i][1] ][2] );

                data.push_back( verts[ inds[i][2] ][0] );
                data.push_back( verts[ inds[i][2] ][1] );
                data.push_back( verts[ inds[i][2] ][2] );
                data.push_back( norms[ inds[i][3] ][0] );
                data.push_back( norms[ inds[i][3] ][1] );
                data.push_back( norms[ inds[i][3] ][2] );

                data.push_back( verts[ inds[i][4] ][0] );
                data.push_back( verts[ inds[i][4] ][1] );
                data.push_back( verts[ inds[i][4] ][2] );
                data.push_back( norms[ inds[i][5] ][0] );
                data.push_back( norms[ inds[i][5] ][1] );
                data.push_back( norms[ inds[i][5] ][2] );
            }
            //data[] has now the form : {x1,y1,z1, nx1,ny1,nz1, x2,y2,z2, nx2,ny2,nz2 ... }

            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, data.size()*sizeof(float), &data[0], GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
            glEnableVertexAttribArray(1);
            nverts = (int)(data.size()/6); //number of vertices to draw
        }
        else
        {
            printf("Invalid arguments in mesh() constructor. Exiting...\n");
            exit(EXIT_FAILURE);
        }
    }

    //delete the mesh
    ~mesh()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

    //draw the mesh (points)
    void draw_points(float point_size = 1.0f)
    {
        glPointSize(point_size);
        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, nverts);
        glBindVertexArray(0);
    }

    //draw the mesh (triangles)
    void draw_triangles()
    {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, nverts);
        glBindVertexArray(0);
    }
};
#endif
