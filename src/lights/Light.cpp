#include "Light.h"

Light::Light() {
  color = glm::vec3(1.0f, 1.0f, 1.0f);
  ambientIntensity = 1.0f;
  diffuseIntensity = 0.0f;
}

Light::Light(glm::vec3 pColor, float aIntensity, float dIntensity)
    : color{pColor} {
  ambientIntensity = aIntensity;
  diffuseIntensity = dIntensity;
}

Light::~Light() {}
