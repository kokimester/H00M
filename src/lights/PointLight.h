#pragma once
#include "Light.h"
#include <glad/glad.h>
class PointLight : public Light {
protected:
  glm::vec3 position;

  float constant, linear, exponent;

public:
  PointLight();
  PointLight(const glm::vec3 &pColor, float aIntensity, float dIntensity,
             float xPos, float yPos, float zPos, float pConst, float pLinear,
             float pExp);
  ~PointLight() {}

  glm::vec3 getPos() const { return position; }
  float getConst() const { return constant; }
  float getLinear() const { return linear; }
  float getExp() const { return exponent; }

  void useLight(uint ambientIntensityLocation, uint ambientColorLocation,
                uint diffuseIntensityLocation, uint positionLocation,
                uint constantLocation, uint linearLocation,
                uint exponentLocation);
};
