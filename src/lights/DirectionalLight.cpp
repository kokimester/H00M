#include "DirectionalLight.h"

DirectionalLight::DirectionalLight() : Light() {
  direction = glm::vec3(0.0f, -1.0f, 0.0f);
}

DirectionalLight::DirectionalLight(float red, float green, float blue,
                                   float aIntensity, float dIntensity,
                                   float xDir, float yDir, float zDir)
    : Light(red, green, blue, aIntensity, dIntensity) {
  direction = glm::vec3(xDir, yDir, zDir);
}

void DirectionalLight::useLight(uint ambientIntensityLocation,
                                uint ambientColourLocation,
                                uint diffuseIntensityLocation,
                                uint directionLocation) {
  glUniform3f(ambientColourLocation, colour.x, colour.y, colour.z);
  glUniform1f(ambientIntensityLocation, ambientIntensity);
  glUniform3f(directionLocation, direction.x, direction.y, direction.z);
  glUniform1f(diffuseIntensityLocation, diffuseIntensity);
}
