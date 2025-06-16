#pragma once
#include "PointLight.h"

class Spotlight : public PointLight {
private:
  glm::vec3 direction;

  GLfloat edge, procEdge; // egy szog ami megmondja meddig tart a feny
  GLfloat innerCutoff, outerCutoff;

public:
  Spotlight();
  Spotlight(const glm::vec3 &pColor, GLfloat aIntensity, GLfloat dIntensity,
            GLfloat xPos, GLfloat yPos, GLfloat zPos, GLfloat xDir,
            GLfloat yDir, GLfloat zDir, GLfloat pConst, GLfloat pLinear,
            GLfloat pExp, GLfloat pEdge, GLfloat pInner, GLfloat pOuter);

  glm::vec3 getDirection() const { return direction; }
  GLfloat getEdge() const { return edge; }
  GLfloat getProcEdge() const { return procEdge; }
  GLfloat getInnerCutoff() const { return innerCutoff; }
  GLfloat getOuterCutoff() const { return outerCutoff; }

  void update(glm::vec3 newPos, glm::vec3 newDir,
              [[maybe_unused]] glm::vec3 rightDir) {
    PointLight::position = newPos + glm::vec3(0.0f, -0.3f, 0.0f);
    direction = newDir;
  }

  void disable() { direction = glm::vec3(0.0f, 1.0f, 0.0f); }

  void useLight(GLuint ambientIntensityLocation, GLuint ambientColorLocation,
                GLuint diffuseIntensityLocation, GLuint positionLocation,
                GLuint directionLocation, GLuint constantLocation,
                GLuint linearLocation, GLuint exponentLocation,
                GLuint edgeLocation, GLuint innerCutoffLocation,
                GLuint outerCutoffLocation);

  ~Spotlight() {}
};
