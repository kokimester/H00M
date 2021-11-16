#pragma once
#include "Light.h"
class PointLight :
    public Light
{
protected:
    glm::vec3 position;

    GLfloat constant, linear, exponent;
public:
    PointLight();
    PointLight(GLfloat red, GLfloat green, GLfloat blue,
        GLfloat aIntensity, GLfloat dIntensity,
        GLfloat xPos, GLfloat yPos, GLfloat zPos,
        GLfloat pConst, GLfloat pLinear, GLfloat pExp);
    ~PointLight() {}

    glm::vec3 getPos() const { return position; }
    GLfloat getConst() const { return constant; }
    GLfloat getLinear() const { return linear; }
    GLfloat getExp() const { return exponent; }

    void useLight(GLuint ambientIntensityLocation, GLuint ambientColourLocation,
        GLuint diffuseIntensityLocation, GLuint positionLocation,
        GLuint constantLocation, GLuint linearLocation, GLuint exponentLocation);
};

