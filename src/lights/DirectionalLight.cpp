#include "DirectionalLight.h"

DirectionalLight::DirectionalLight() : Light() {
  direction = glm::vec3(0.0f, -1.0f, 0.0f);
}

DirectionalLight::DirectionalLight(const glm::vec3 &color, float aIntensity,
                                   float dIntensity,
                                   const glm::vec3 &pDirection)
    : Light(color, aIntensity, dIntensity), direction{pDirection} {}

void DirectionalLight::useLight(unsigned int ambientIntensityLocation,
                                unsigned int ambientColorLocation,
                                unsigned int diffuseIntensityLocation,
                                unsigned int directionLocation) {
  glUniform3f(ambientColorLocation, color.x, color.y, color.z);
  glUniform1f(ambientIntensityLocation, ambientIntensity);
  glUniform3f(directionLocation, direction.x, direction.y, direction.z);
  glUniform1f(diffuseIntensityLocation, diffuseIntensity);
}
