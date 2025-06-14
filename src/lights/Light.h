#pragma once

/* #include <GL/glew.h> */
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Light {
protected:
  glm::vec3 colour;
  float ambientIntensity;
  float diffuseIntensity;

public:
  Light();
  Light(float red, float green, float blue, float aIntensity, float dIntensity);
  ~Light();

  glm::vec3 getColour() const { return colour; }
  float getAmbientIntensity() const { return ambientIntensity; }
  float getDiffuseIntensity() const { return diffuseIntensity; }
};
