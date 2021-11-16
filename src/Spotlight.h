#pragma once
#include "PointLight.h"


class Spotlight :
    public PointLight
{
private:
    glm::vec3 direction;

    GLfloat edge, procEdge; // egy szog ami megmondja meddig tart a feny
public:
    Spotlight();
    Spotlight(GLfloat red, GLfloat green, GLfloat blue,
        GLfloat aIntensity, GLfloat dIntensity,
        GLfloat xPos, GLfloat yPos, GLfloat zPos,
        GLfloat xDir, GLfloat yDir, GLfloat zDir,
        GLfloat pConst, GLfloat pLinear, GLfloat pExp,
        GLfloat pEdge);

    glm::vec3 getDirection() const { return direction; }
    GLfloat getEdge() const { return edge; }
    GLfloat getProcEdge() const { return procEdge; }

    void update(glm::vec3 newPos, glm::vec3 newDir, glm::vec3 rightDir)
    { 
        PointLight::position = newPos + glm::vec3(0.0f,-0.3f,0.0f);
       direction = newDir;
    }

    void disable() { direction = glm::vec3(0.0f, 1.0f, 0.0f); }

    void useLight(GLuint ambientIntensityLocation, GLuint ambientColourLocation,
        GLuint diffuseIntensityLocation, GLuint positionLocation, GLuint directionLocation,
        GLuint constantLocation, GLuint linearLocation, GLuint exponentLocation,
        GLuint edgeLocation);

    ~Spotlight() {}
    
};