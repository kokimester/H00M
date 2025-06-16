#include "Spotlight.h"

Spotlight::Spotlight() : PointLight() {
  direction = glm::vec3(0.0f, -1.0f, 0.0f);
  edge = 0.0f;
  procEdge = cosf(glm::radians(edge));
}

Spotlight::Spotlight(const glm::vec3 &pColor, GLfloat aIntensity,
                     GLfloat dIntensity, GLfloat xPos, GLfloat yPos,
                     GLfloat zPos, GLfloat xDir, GLfloat yDir, GLfloat zDir,
                     GLfloat pConst, GLfloat pLinear, GLfloat pExp,
                     GLfloat pEdge, GLfloat pInner, GLfloat pOuter)
    : PointLight(pColor, aIntensity, dIntensity, xPos, yPos, zPos, pConst,
                 pLinear, pExp),
      edge{pEdge}, innerCutoff{pInner}, outerCutoff{pOuter} {
  direction = glm::normalize(glm::vec3(xDir, yDir, zDir));
  procEdge = cosf(glm::radians(edge));
}

void Spotlight::useLight(GLuint ambientIntensityLocation,
                         GLuint ambientColorLocation,
                         GLuint diffuseIntensityLocation,
                         GLuint positionLocation, GLuint directionLocation,
                         GLuint constantLocation, GLuint linearLocation,
                         GLuint exponentLocation, GLuint edgeLocation,
                         GLuint innerCutoffLocation,
                         GLuint outerCutoffLocation) {
  glUniform3f(ambientColorLocation, color.x, color.y, color.z);
  glUniform1f(ambientIntensityLocation, ambientIntensity);
  glUniform1f(diffuseIntensityLocation, diffuseIntensity);

  glUniform3f(positionLocation, position.x, position.y, position.z);

  glUniform1f(constantLocation, constant);
  glUniform1f(linearLocation, linear);
  glUniform1f(exponentLocation, exponent);

  glUniform3f(directionLocation, direction.x, direction.y, direction.z);
  glUniform1f(edgeLocation, procEdge);
  glUniform1f(innerCutoffLocation, innerCutoff);
  glUniform1f(outerCutoffLocation, outerCutoff);
}
