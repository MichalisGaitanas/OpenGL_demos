#ifndef MESH_HPP
#define MESH_HPP

#include<GL/glew.h>
#include<iostream>
#include<string>
#include<fstream>
#include<vector>
//#include<unordered_map>

#define STB_IMAGE_IMPLEMENTATION //This must happen only once.
#include"stb_image.h"

class meshvf
{
private:
    unsigned int vao, vbo, ebo; //Vertex array object, vertex buffer object, element (index) buffer object.
    std::vector<float> verts; //Obj's vertices {x1,y1,z1, x2,y2,z2, ...}.
    std::vector<unsigned int> inds; //Obj's indices {i11,i12,i13, i21,i22,i23, ...}.

public:
    meshvf(const char *obj_path)
    {
        std::ifstream fp;
        fp.open(obj_path);
        if (!fp.is_open())
        {
            fprintf(stderr, "Error : File '%s' was not found. Exiting...\n", obj_path);
            exit(EXIT_FAILURE);
        }

        float x,y,z;
        unsigned int i1,i2,i3;
        std::string line;
        while (getline(fp, line))
        {
            if (line[0] == 'v' && line[1] == ' ') //Then we have a vertex line.
            {
                const char *temp_line = line.c_str();
                sscanf(temp_line, "v %f %f %f", &x,&y,&z);
                verts.push_back(x);
                verts.push_back(y);
                verts.push_back(z);
            }
            else if (line[0] == 'f') //Then we have an index line.
            {
                const char *temp_line = line.c_str();
                sscanf(temp_line, "f %u %u %u", &i1,&i2,&i3);
                //Append indices to inds and subtract 1 from each, to convert to 0-based indexing. Obj files are 1-based.
                inds.push_back(i1-1);
                inds.push_back(i2-1);
                inds.push_back(i3-1);
            }
        }

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo); //OpenGL expects the indices stored in the ebo to reference positions in the vertex buffer.
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), &verts[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size()*sizeof(unsigned int), &inds[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    //Cleanup memory.
    ~meshvf()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
    }

    //Draw the mesh in the form of individual triangles.
    void draw_triangles()
    {
        glBindVertexArray(vao); //Bind the mesh's vao.
        glDrawElements(GL_TRIANGLES, (int)inds.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0); //Unbind the vao.
    }

    //Draw the mesh in the form of individual lines (wireframe).
    void draw_lines(const float line_width = 1.0f)
    {
        glBindVertexArray(vao);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Switch to line mode for wireframe/edge only drawing.
        glLineWidth(line_width);
        glDrawElements(GL_TRIANGLES, (int)inds.size(), GL_UNSIGNED_INT, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //Restore fill mode.
        glBindVertexArray(0);
    }

    //Draw the mesh in the form of individual points (vertices).
    void draw_points(const float point_size = 2.0f)
    {
        glBindVertexArray(vao);
        glPointSize(point_size);
        glDrawElements(GL_POINTS, (int)inds.size(), GL_UNSIGNED_INT, 0); //Point mode.
        glBindVertexArray(0);
    }
};



class meshvfn
{
private:
    unsigned int vao, vbo; //Vertex array and buffer object.
    std::vector<std::vector<float>> verts; //Vertices (format : v x y z).
    std::vector<std::vector<float>> norms; //Normals (format : vn nx ny nz).
    std::vector<std::vector<unsigned int>> inds; //Indices (format : f i11//i12 i21//i22 i31//i32).
    std::vector<float> main_buffer; //Final form of the geometry data to draw.

public:
    meshvfn(const char *obj_path)
    {
        std::ifstream fp;
        fp.open(obj_path);
        if (!fp.is_open())
        {
            fprintf(stderr, "Error : File '%s' was not found. Exiting...\n", obj_path);
            exit(EXIT_FAILURE);
        }

        float x,y,z;
        float nx,ny,nz;
        unsigned int i11,i12, i21,i22, i31,i32;
        std::string line;
        while (getline(fp, line))
        {
            if (line[0] == 'v' && line[1] == ' ') //Then we have a vertex line.
            {
                const char *temp_line = line.c_str();
                sscanf(temp_line, "v %f %f %f", &x,&y,&z);
                verts.push_back({x,y,z});
            }
            else if (line[0] == 'v' && line[1] == 'n') //Then we have a normal line.
            {
                const char *temp_line = line.c_str();
                sscanf(temp_line, "vn %f %f %f", &nx,&ny,&nz);
                norms.push_back({nx,ny,nz});
            }
            else if (line[0] == 'f') //Then we have an index line.
            {
                const char *temp_line = line.c_str();
                sscanf(temp_line, "f %u//%u %u//%u %u//%u", &i11,&i12, &i21,&i22, &i31,&i32);
                //Append indices to inds and subtract 1 from each, to convert to 0-based indexing. Obj files are 1-based.
                inds.push_back({i11-1, i12-1, i21-1, i22-1, i31-1, i32-1});
            }
        }

        //Now combine verts[][], norms[][] and inds[][] to construct the main buffer main_buffer[],
        //which will have everything needed in the correct order for rendering.
        for (size_t i = 0; i < inds.size(); ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                main_buffer.push_back( verts[ inds[i][2*j] ][0] );
                main_buffer.push_back( verts[ inds[i][2*j] ][1] );
                main_buffer.push_back( verts[ inds[i][2*j] ][2] );

                main_buffer.push_back( norms[ inds[i][2*j+1] ][0] );
                main_buffer.push_back( norms[ inds[i][2*j+1] ][1] );
                main_buffer.push_back( norms[ inds[i][2*j+1] ][2] );
            }
        }
        //main_buffer[] has now the form : {x1,y1,z1, nx1,ny1,nz1, x2,y2,z2, nx2,ny2,nz2 ... }.
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, main_buffer.size()*sizeof(float), &main_buffer[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    //Delete the mesh's memory resources.
    ~meshvfn()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

    //Draw the mesh (triangles).
    void draw_triangles()
    {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, (int)main_buffer.size()/6);
        glBindVertexArray(0);
    }
};

/*
class meshvfn_ebo
{
private:
    unsigned int vao, vbo, ebo; // Vertex Array, Vertex Buffer, and Element Buffer Objects
    std::vector<float> main_buffer; // Interleaved vertex and normal data
    std::vector<unsigned int> indices; // Element buffer indices

public:
    meshvfn_ebo(const char *obj_path)
    {
        std::ifstream fp;
        fp.open(obj_path);
        if (!fp.is_open())
        {
            fprintf(stderr, "Error : File '%s' was not found. Exiting...\n", obj_path);
            exit(EXIT_FAILURE);
        }

        // Temporary storage for vertices, normals, and indices
        std::vector<std::vector<float>> verts; // {x, y, z}
        std::vector<std::vector<float>> norms; // {nx, ny, nz}
        std::unordered_map<std::string, unsigned int> uniqueVertexMap; // To store unique vertex-normal combos

        float x, y, z;
        float nx, ny, nz;
        unsigned int vi1, vi2, vi3, ni1, ni2, ni3;
        std::string line;

        while (getline(fp, line))
        {
            if (line[0] == 'v' && line[1] == ' ') // Vertex line
            {
                sscanf(line.c_str(), "v %f %f %f", &x, &y, &z);
                verts.push_back({x, y, z});
            }
            else if (line[0] == 'v' && line[1] == 'n') // Normal line
            {
                sscanf(line.c_str(), "vn %f %f %f", &nx, &ny, &nz);
                norms.push_back({nx, ny, nz});
            }
            else if (line[0] == 'f') // Face line
            {
                sscanf(line.c_str(), "f %u//%u %u//%u %u//%u", &vi1, &ni1, &vi2, &ni2, &vi3, &ni3);

                processVertex(verts, norms, vi1-1, ni1-1, uniqueVertexMap); // vi1, ni1
                processVertex(verts, norms, vi2-1, ni2-1, uniqueVertexMap); // vi2, ni2
                processVertex(verts, norms, vi3-1, ni3-1, uniqueVertexMap); // vi3, ni3
            }
        }

        // Generate VAO, VBO, and EBO
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, main_buffer.size() * sizeof(float), &main_buffer[0], GL_STATIC_DRAW);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0); // Unbind VAO
    }

    // Destructor to cleanup OpenGL resources
    ~meshvfn_ebo()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
    }

    // Draw the mesh using glDrawElements
    void draw_triangles()
    {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, (int)indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0); // Unbind VAO
    }

private:
    // Helper function to process each vertex-normal pair
    void processVertex(
        const std::vector<std::vector<float>>& verts,
        const std::vector<std::vector<float>>& norms,
        unsigned int vertexIndex,
        unsigned int normalIndex,
        std::unordered_map<std::string, unsigned int>& uniqueVertexMap)
    {
        // Create a key for the current vertex-normal pair
        std::string key = std::to_string(vertexIndex) + "//" + std::to_string(normalIndex);

        if (uniqueVertexMap.find(key) == uniqueVertexMap.end()) {
            // This is a new vertex-normal combination, so store it
            main_buffer.push_back(verts[vertexIndex][0]);
            main_buffer.push_back(verts[vertexIndex][1]);
            main_buffer.push_back(verts[vertexIndex][2]);
            main_buffer.push_back(norms[normalIndex][0]);
            main_buffer.push_back(norms[normalIndex][1]);
            main_buffer.push_back(norms[normalIndex][2]);

            // Assign a new index for this unique vertex-normal combo
            unsigned int newIndex = (unsigned int)(main_buffer.size() / 6 - 1);
            uniqueVertexMap[key] = newIndex;
            indices.push_back(newIndex);
        } else {
            // This vertex-normal pair already exists, use its existing index
            indices.push_back(uniqueVertexMap[key]);
        }
    }
};
*/

class meshvft
{
private:
    unsigned int vao, vbo, tao; //Vertex array, buffer and texture 'array object' (there's no tao officialy, but the name suits well to vao and vbo).
    std::vector<std::vector<float>> verts; //Vertices (format : v x y z)
    std::vector<std::vector<float>> uvs; //Texture (format : vt u v).
    std::vector<std::vector<unsigned int>> inds; //Indices (format : f i11/i12 i21/i22 i31/i32.
    std::vector<float> main_buffer; //Final form of the geometry data to draw.

public:

    meshvft(const char *obj_path, const char *img_path)
    {
        std::ifstream fp;
        fp.open(obj_path);
        if (!fp.is_open())
        {
            fprintf(stderr, "Error : File '%s' was not found. Exiting...\n", obj_path);
            exit(EXIT_FAILURE);
        }

        float x,y,z;
        float u,v;
        unsigned int i11,i12, i21,i22, i31,i32;
        std::string line;
        while (getline(fp, line))
        {
            if (line[0] == 'v' && line[1] == ' ') //Then we have a vertex line.
            {
                const char *tmp_line = line.c_str();
                sscanf(tmp_line, "v %f %f %f", &x,&y,&z);
                verts.push_back({x,y,z});
            }
            else if (line[0] == 'v' && line[1] == 't') //Then we have a texture coordinate (uv) line.
            {
                const char *tmp_line = line.c_str();
                sscanf(tmp_line, "vt %f %f", &u,&v);
                uvs.push_back({u,v});
            }
            else if (line[0] == 'f') //Then we have a index line.
            {
                const char *tmp_line = line.c_str();
                sscanf(tmp_line, "f %u/%u %u/%u %u/%u", &i11,&i12, &i21,&i22, &i31,&i32);
                //Append indices to inds and subtract 1 from each, to convert to 0-based indexing. Obj files are 1-based.
                inds.push_back({i11-1, i12-1, i21-1, i22-1, i31-1, i32-1});
            }
        }

        //Now combine verts[][], uvs[][] and inds[][] to construct the main buffer main_buffer[]
        //which will have all the main_buffer needed for drawing.
        for (size_t i = 0; i < inds.size(); ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                main_buffer.push_back( verts[ inds[i][2*j] ][0] );
                main_buffer.push_back( verts[ inds[i][2*j] ][1] );
                main_buffer.push_back( verts[ inds[i][2*j] ][2] );

                main_buffer.push_back( uvs[ inds[i][2*j+1] ][0] );
                main_buffer.push_back( uvs[ inds[i][2*j+1] ][1] );
            }
        }
        //main_buffer[] has now the form : {x1,y1,z1, u1,v1, x2,y2,z2, u2,v2 ... }

        //Tell OpenGL how to process the mesh data in the gpu.
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, main_buffer.size()*sizeof(float), &main_buffer[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);

        //Tell OpenGL how to apply the texture on the mesh.
        glGenTextures(1, &tao);
        glBindTexture(GL_TEXTURE_2D, tao);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //This is useful for textures with non-standard widths or single-channel textures (e.g. grayscale).

        //Load the image texture.
        int img_width, img_height, img_channels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char *img_data = stbi_load(img_path, &img_width, &img_height, &img_channels, 0);
        if (!img_data)
        {
            fprintf(stderr, "Error : File '%s' was not found. Exiting...\n", img_path);
            exit(EXIT_FAILURE);
        }

        //Determine the correct format based on the number of channels (img_channels).
        GLenum format;
        if (img_channels == 1)
            format = GL_RED; //Single-channel (grayscale image).
        else if (img_channels == 3)
            format = GL_RGB; //Classical 3-channel image (e.g. jpg).
        else if (img_channels == 4)
            format = GL_RGBA; //4-channel image, i.e. RGB + alpha channel for opacity (e.g. png).

        glTexImage2D(GL_TEXTURE_2D, 0, format, img_width, img_height, 0, format, GL_UNSIGNED_BYTE, img_data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(img_data); //Free image resources.
    }

    //Cleanup memory.
    ~meshvft()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteTextures(1, &tao);
    }

    //Draw the mesh (triangles).
    void draw_triangles()
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tao);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, (int)main_buffer.size()/5);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};



class skybox
{
private:
    unsigned int vao, vbo, ebo, tao;

public:
    //Make sure that the images have all the same size in pixels (e.g. 2048x2048, 500x500, etc...) AND channels.
    skybox(const char *right_img_path, const char *left_img_path, const char *top_img_path, const char *bottom_img_path, const char *front_img_path, const char *back_img_path)
    {   
        /* Basically a cube, procedurally generated. */

        float verts[] = { -1.0f, -1.0f,  1.0f,
                           1.0f, -1.0f,  1.0f,
                           1.0f, -1.0f, -1.0f,
                          -1.0f, -1.0f, -1.0f,
                          -1.0f,  1.0f,  1.0f,
                           1.0f,  1.0f,  1.0f,
                           1.0f,  1.0f, -1.0f,
                          -1.0f,  1.0f, -1.0f };
      
        unsigned int inds[] = { //Right.
                                1, 2, 6,
                                6, 5, 1,
                                //Left.
                                0, 4, 7,
                                7, 3, 0,
                                //Top.
                                4, 5, 6,
                                6, 7, 4,
                                //Bottom.
                                0, 3, 2,
                                2, 1, 0,
                                //Front.
                                3, 7, 6,
                                6, 2, 3, 
                                //Back.
                                0, 1, 5,
                                5, 4, 0 };

        //Setup skybox's data in the memory.
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), &verts, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(inds), &inds, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        //Create the skybox's texture.
        glGenTextures(1, &tao);
        glBindTexture(GL_TEXTURE_CUBE_MAP, tao);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //This is useful for textures with non-standard widths or single-channel textures (e.g. grayscale).
        //glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        //Skybox's expected image names. Do not change their order!
        std::string paths[6] = { right_img_path, left_img_path, top_img_path, bottom_img_path, front_img_path, back_img_path };
        int img_widths[6], img_heights[6], img_channels[6];

        stbi_set_flip_vertically_on_load(false);
        for (int i = 0; i < 6; i++)
        {
            unsigned char *data = stbi_load(paths[i].c_str(), &img_widths[i], &img_heights[i], &img_channels[i], 0);
            if (!data)
                fprintf(stderr, "Error : Failed to load texture '%s'\n", paths[i].c_str());

            //Determine the correct format for glTexImage2D based on the number of channels (img_channels).
            GLenum format;
            if (img_channels[i] == 1)
                format = GL_RED; //Single-channel grayscale image.
            else if (img_channels[i] == 3)
                format = GL_RGB; //Classical 3-channel image (e.g. jpg).
            else if (img_channels[i] == 4)
                format = GL_RGBA; //4-channel image, i.e. RGB + alpha channel for opacity (e.g. png).

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, img_widths[i], img_heights[i], 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }

        //Check if all images have the same width, height, and channels. Otherwise the skybox may not render.
        bool img_consistency = true;
        for (int i = 1; i < 6; i++)
        {
            if (img_widths[i] != img_widths[0] || img_heights[i] != img_heights[0] || img_channels[i] != img_channels[0])
            {
                img_consistency = false;
                break;
            }
        }
        if (!img_consistency)
            fprintf(stderr, "Error : All 6 images must have the same width, height, and channels.\n");

    }

    //Delete the skybox's resources.
    ~skybox()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &ebo);
        glDeleteBuffers(1, &vbo);
        glDeleteTextures(1, &tao);
    }

    //Draw the skybox cube mesh.
    void draw_elements()
    {
        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, tao);
        glBindVertexArray(vao);
        glDepthFunc(GL_LEQUAL); //Ensures that the skybox fragments will render behind everything else. (A bit dangerous to place it here. Be cautious.)
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glDepthFunc(GL_LESS); //Restore the default depth test function for rendering the rest of the scene.
		glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
};



class quadtex
{
private:
    unsigned int vao, vbo;

public:
    quadtex()
    {
        //Procedural.
        float main_buffer[] = {  //Positions.         //UVs.
                                 -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
                                 -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
                                  1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
                                  
                                 -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
                                  1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
                                  1.0f,  1.0f, 0.0f,  1.0f, 1.0f };

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(main_buffer), &main_buffer, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0); //Positions.
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float))); //UVs.
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    //Delete the quadtex mesh.
    ~quadtex()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

    //Draw the quadtex mesh (triangles).
    void draw_triangles(unsigned int framebuffer_tao)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, framebuffer_tao);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};

#endif
