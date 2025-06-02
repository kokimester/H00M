#pragma once

/* #include <GLES3/gl3.h> */
#include <glad/glad.h>

class Material {
private:
  float specularIntensity;
  float shininess;

public:
  Material();
  Material(float sIntensity, float shine);
  ~Material() {};

  float getShininess() const { return shininess; }
  float getSpecularIntensity() const { return specularIntensity; }

  void useMaterial(unsigned int specularIntensityLocation,
                   unsigned int shininessLocation) {
    glUniform1f(specularIntensityLocation, specularIntensity);
    glUniform1f(shininessLocation, shininess);
  }
};
