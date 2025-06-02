#include "PointLight.h"

PointLight::PointLight() : Light() {
  position = glm::vec3(0.0f, 0.0f, 0.0f);
  constant = 1.0f;
  linear = 0.0f;
  exponent = 0.0f;
}

PointLight::PointLight(float red, float green, float blue, float aIntensity,
                       float dIntensity, float xPos, float yPos, float zPos,
                       float pConst, float pLinear, float pExp)
    : Light(red, green, blue, aIntensity, dIntensity) {
  position = glm::vec3(xPos, yPos, zPos);
  constant = pConst;
  linear = pLinear;
  exponent = pExp;
}

void PointLight::useLight(uint ambientIntensityLocation,
                          uint ambientColourLocation,
                          uint diffuseIntensityLocation, uint positionLocation,
                          uint constantLocation, uint linearLocation,
                          uint exponentLocation) {
  glUniform3f(ambientColourLocation, colour.x, colour.y, colour.z);
  glUniform1f(ambientIntensityLocation, ambientIntensity);
  glUniform1f(diffuseIntensityLocation, diffuseIntensity);

  glUniform3f(positionLocation, position.x, position.y, position.z);

  glUniform1f(constantLocation, constant);
  glUniform1f(linearLocation, linear);
  glUniform1f(exponentLocation, exponent);
}
