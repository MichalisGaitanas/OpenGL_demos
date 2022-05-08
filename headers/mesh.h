#ifndef MESH_H
#define MESH_H

#include<GL/glew.h>
#include<iostream>
#include<fstream>
#include<vector>
using namespace std;

class mesh
{
public:
    unsigned int vao,vbo; //vertex array and buffer object
    vector<float> data; //final form of the geometry data to draw
    unsigned int nVerts; //number of vertices to draw

    mesh(const char *path, int loadVerts, int loadFaces, int loadNorms)
    {
        ifstream fdata;
        fdata.open(path);
        if (!fdata.is_open())
        {
            cout << path << " not found. Exiting...\n";
            exit(EXIT_FAILURE);
        }

        if (loadVerts == 1 && loadFaces == 0 && loadNorms == 0) //load vertices
        {
            string line;
            int iVertex = 0;

            //vertices (format : v x y z)
            vector<vector<float>> verts;
            float x,y,z;
            vector<float> vertsRow;

            while (getline(fdata, line))
            {
                if (line[0] == 'v' && line[1] == ' ') //then we have a vertex line
                {
                    const char *tmpLine = line.c_str();
                    sscanf(tmpLine, "v %f %f %f", &x,&y,&z);
                    verts.push_back(vertsRow);
                    verts[iVertex].push_back(x);
                    verts[iVertex].push_back(y);
                    verts[iVertex].push_back(z);
                    iVertex++;
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
            nVerts = data.size(); //number of vertices to draw
        }
        else if (loadVerts == 1 && loadFaces == 1 && loadNorms == 0) //load vertices and faces
        {
            string line;
            int iVertex = 0, iFace = 0;

            //vertices (format : v x y z)
            vector<vector<float>> verts;
            float x,y,z;
            vector<float> vertsRow;

            //indices (format : f i11//i12 i21//i22 i31//i32)
            vector<vector<unsigned int>> inds;
            unsigned int i11,i12, i21,i22, i31,i32;
            vector<unsigned int> indsRow;

            while (getline(fdata, line))
            {
                if (line[0] == 'v' && line[1] == ' ') //then we have a vertex line
                {
                    const char *tmpLine = line.c_str();
                    sscanf(tmpLine, "v %f %f %f", &x,&y,&z);
                    verts.push_back(vertsRow);
                    verts[iVertex].push_back(x);
                    verts[iVertex].push_back(y);
                    verts[iVertex].push_back(z);
                    iVertex++;
                }
                else if (line[0] == 'f') //then we have an indices line
                {
                    const char *tmpLine = line.c_str();
                    sscanf(tmpLine, "f %d//%d %d//%d %d//%d", &i11,&i12, &i21,&i22, &i31,&i32);
                    inds.push_back(indsRow);
                    inds[iFace].push_back(i11-1);
                    inds[iFace].push_back(i21-1);
                    inds[iFace].push_back(i31-1);
                    iFace++;
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
            nVerts = (int)(data.size()/3); //number of vertices to draw
        }
        else if (loadVerts == 1 && loadFaces == 1 && loadNorms == 1) //load vertices, faces and normals
        {
            string line;
            int iVertex = 0, iNormal = 0, iFace = 0;

            //vertices (format : v x y z)
            vector<vector<float>> verts;
            float x,y,z;
            vector<float> vertsRow;

            //norms (format : vn nx ny nz)
            vector<vector<float>> norms;
            float nx,ny,nz;
            vector<float> normsRow;

            //indices (format : f i11//i12 i21//i22 i31//i32)
            vector<vector<unsigned int>> inds;
            unsigned int i11,i12, i21,i22, i31,i32;
            vector<unsigned int> indsRow;

            while (getline(fdata, line))
            {
                if (line[0] == 'v' && line[1] == ' ') //then we have a vertex line
                {
                    const char *tmpLine = line.c_str();
                    sscanf(tmpLine, "v %f %f %f", &x,&y,&z);
                    verts.push_back(vertsRow);
                    verts[iVertex].push_back(x);
                    verts[iVertex].push_back(y);
                    verts[iVertex].push_back(z);
                    iVertex++;
                }
                else if (line[0] == 'v' && line[1] == 'n') //then we have a normal line
                {
                    const char *tmpLine = line.c_str();
                    sscanf(tmpLine, "vn %f %f %f", &nx,&ny,&nz);
                    norms.push_back(normsRow);
                    norms[iNormal].push_back(nx);
                    norms[iNormal].push_back(ny);
                    norms[iNormal].push_back(nz);
                    iNormal++;
                }
                else if (line[0] == 'f') //then we have a indices line
                {
                    const char *tmpLine = line.c_str();
                    sscanf(tmpLine, "f %d//%d %d//%d %d//%d", &i11,&i12, &i21,&i22, &i31,&i32);

                    inds.push_back(indsRow);
                    inds[iFace].push_back(i11-1);
                    inds[iFace].push_back(i12-1);
                    inds[iFace].push_back(i21-1);
                    inds[iFace].push_back(i22-1);
                    inds[iFace].push_back(i31-1);
                    inds[iFace].push_back(i32-1);
                    iFace++;
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
            nVerts = (int)(data.size()/6); //number of vertices to draw
        }
        else
        {
            cout << "Invalid arguments in mesh() constructor. Exiting...\n";
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
    void draw_points(float pointSize = 1.0f)
    {
        glPointSize(pointSize);
        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, nVerts);
        glBindVertexArray(0);
    }

    //draw the mesh (triangles)
    void draw_triangles()
    {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, nVerts);
        glBindVertexArray(0);
    }
};
#endif
