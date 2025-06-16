#pragma once
#include "Light.h"
/* #include <GLES3/gl3.h> */
#include <glad/glad.h>

class DirectionalLight : public Light {
private:
  glm::vec3 direction;

public:
  DirectionalLight();
  DirectionalLight(const glm::vec3 &color, float aIntensity, float dIntensity,
                   const glm::vec3 &direction);

  glm::vec3 getDirection() const { return direction; }

  void useLight(uint ambientIntensityLocation, uint ambientColorLocation,
                uint diffuseIntensityLocation, uint directionLocation);

  ~DirectionalLight() {};
};
