#pragma once
#include "Light.h"
/* #include <GLES3/gl3.h> */
#include <glad/glad.h>

class DirectionalLight : public Light {
private:
  glm::vec3 direction;

public:
  DirectionalLight();
  DirectionalLight(float red, float green, float blue, float aIntensity,
                   float dIntensity, float xDir, float yDir, float zDir);

  glm::vec3 getDirection() const { return direction; }

  void useLight(uint ambientIntensityLocation, uint ambientColourLocation,
                uint diffuseIntensityLocation, uint directionLocation);

  ~DirectionalLight() {};
};
