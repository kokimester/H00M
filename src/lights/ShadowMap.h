#pragma once
#include "Entity.h"
#include "Shader.h"

class ShadowMap {
public:
  unsigned int depthMapFBO                    = 0;
  unsigned int depthMapTextureID              = 0;
  static constexpr unsigned int SHADOW_WIDTH  = 4096;
  static constexpr unsigned int SHADOW_HEIGHT = 4096;

public:
  virtual ~ShadowMap() = default;
  void setup();
  void clear();
  void render(model_system& ms, registry& componentRegistry,
              Shader& shadow_shader, Shader& normal_shader,
              const glm::mat4& lightSpaceMatrix);
};
