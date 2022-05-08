#ifndef CAMERA_H
#define CAMERA_H

#include<GL/glew.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

class camera
{
public:
    glm::vec3 pos; //position vector
    glm::vec3 right, front, up; //coordinate system
    glm::vec3 worldUp; //up direction
    float yaw, pitch; // yaw, pitch angles (roll = 0)
    float vel; //velocity magnitude
    float mouseSensitivity;

    camera(glm::vec3 initPos = glm::vec3(0.0f,0.0f,0.0f), glm::vec3 initWorldUp = glm::vec3(0.0f,0.0f,1.0f),
           float initYaw = 90.0f, float initPitch = 0.0f, float initVel = 10.0f, float initMouseSensitivity = 0.1f)
    {
        pos = initPos;
        worldUp = initWorldUp;
        yaw = initYaw;
        pitch = initPitch;
        vel = initVel;
        mouseSensitivity = initMouseSensitivity;
        update_vectors();
    }

    void translate_front(float deltaTime)
    {
        pos += vel*deltaTime*front;
    }

    void translate_back(float deltaTime)
    {
        pos -= vel*deltaTime*front;
    }

    void translate_right(float deltaTime)
    {
        pos += vel*deltaTime*right;
    }

    void translate_left(float deltaTime)
    {
        pos -= vel*deltaTime*right;
    }

    void rotate(float xoffset, float yoffset)
    {
        if (pitch > 88.9f)
            pitch = 88.9f;
        else if (pitch <= -88.9f)
            pitch = -88.9f;

        if (abs(yaw) > 360.0f)
            yaw = 0.0f;

        yaw -= xoffset*mouseSensitivity;
        pitch -= yoffset*mouseSensitivity;

        update_vectors();
    }

    void update_vectors()
    {
        front.x = cos(glm::radians(pitch))*cos(glm::radians(yaw));
        front.y = cos(glm::radians(pitch))*sin(glm::radians(yaw));
        front.z = sin(glm::radians(pitch));
        front = normalize(front);
        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }

    glm::mat4 view()
    {
        return glm::lookAt(pos, pos + front, up);
    }
};

#endif
