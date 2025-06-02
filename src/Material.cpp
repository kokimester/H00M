#include "Material.h"

Material::Material() : specularIntensity(0.0f), shininess(0.0f) {}

Material::Material(float sIntensity, float shine)
    : specularIntensity(sIntensity), shininess(shine) {}
