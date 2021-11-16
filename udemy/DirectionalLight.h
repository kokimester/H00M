#pragma once
#include "Light.h"
class DirectionalLight :
    public Light
{
private:
    glm::vec3 direction;
public:
    DirectionalLight();
    DirectionalLight(GLfloat red, GLfloat green, GLfloat blue, GLfloat aIntensity, GLfloat dIntensity,
        GLfloat xDir, GLfloat yDir, GLfloat zDir);

    glm::vec3 getDirection() const { return direction; }

    void useLight(GLuint ambientIntensityLocation, GLuint ambientColourLocation,
        GLuint diffuseIntensityLocation, GLuint directionLocation);

    ~DirectionalLight() {};
};

