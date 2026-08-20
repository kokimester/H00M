#pragma once
#include "Line.h"
#include "Material.h"
#include "Model.h"
#include "Shader.h"
#include "Texture.h"
#include <cstddef>
#include <glm/glm.hpp>
#include <unordered_map>

using EntityType = std::size_t;
extern EntityType MAX_ENTITY; // extern, not static

EntityType create_entity();

struct AABB {
  glm::vec3 min, max;
  AABB() : min{0.f}, max{0.f} {}
  AABB(const glm::vec3 min, const glm::vec3 max) : min{min}, max{max} {}
  AABB(const AABB& theOther) = default;
  void move(const glm::vec3& direction) {
    min += direction;
    max += direction;
  }
};

bool doesCollidePointvAABB(const glm::vec3& point, const AABB& box);

bool doesCollideAABBvAABB(const AABB& a, const AABB& b);
glm::vec3 getCollisionDirection(const AABB& a, const AABB& b);

// box, and color can be separated into another struct serving
// as a universal renderable wireframe box
struct collision_component {
  AABB box;
  bool isMovable   = false;
  bool isColliding = false;
  glm::vec3 color  = {1.f, 0.f, 1.f};
  constexpr static glm::vec3 NOT_COLLIDING_COLOR{0.f, 1.f, 0.f};
  constexpr static glm::vec3 COLLIDING_COLOR{1.f, 0.f, 0.f};
};

// TODO: add hitbox rendering
struct hitbox_component {
  AABB box;
  bool isTargeted = false;
  glm::vec3 color = {1.f, 0.f, 1.f};
  constexpr static glm::vec3 NOT_TARGETED_COLOR{1.f, 1.f, 0.f};
  constexpr static glm::vec3 TARGETED_COLOR{1.f, 0.5f, 0.f};
};

struct transform_component {
  glm::vec3 pos{0.f};
  glm::vec3 vel{0.f};
  glm::vec3 acc{0.f};
  glm::vec3 rot{0.f};
  glm::vec3 scale{1.f};
  GLfloat rotationInDegrees = 0.f;
  GLfloat rotationvel       = 0.f;
};

struct model_component {
  glm::mat4 modelMat{1.f};
  glm::mat4 modelDefaultOrientationRotation{1.f};

  Model& model;
  Material& material;
  Texture& texture;
  model_component(Model& model, Material& material, Texture& texture,
                  glm::mat4 defaultRotation = {1.f})
      : modelDefaultOrientationRotation{defaultRotation}, model{model},
        material{material}, texture{texture} {}
};

struct registry {
  std::unordered_map<EntityType, model_component> models;
  std::unordered_map<EntityType, transform_component> transforms;
  std::unordered_map<EntityType, collision_component> collision_boxes;
  std::unordered_map<EntityType, hitbox_component> hitboxes;
};

struct model_system {
  void update(registry& reg);
  void render(registry& reg, Shader& shader);
};

struct transform_system {
  void update(registry& reg, float dt);
};

struct hitbox_system {
  void update(registry& reg, const Line& hit_line);
  void render(registry& reg);
};
