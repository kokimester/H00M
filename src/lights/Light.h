#pragma once

/* #include <GL/glew.h> */
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Light {
protected:
  glm::vec3 color;
  float ambientIntensity;
  float diffuseIntensity;

public:
  Light();
  Light(glm::vec3 pColor, float aIntensity, float dIntensity);
  ~Light();

  glm::vec3 getColor() const { return color; }
  float getAmbientIntensity() const { return ambientIntensity; }
  float getDiffuseIntensity() const { return diffuseIntensity; }
};
