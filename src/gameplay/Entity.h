#pragma once
#include "Material.h"
#include "Model.h"
#include "Shader.h"
#include "Texture.h"
#include <cstddef>
#include <glm/glm.hpp>
#include <unordered_map>

using EntityType             = std::size_t;
static EntityType MAX_ENTITY = 0;
inline EntityType create_entity() {
  static EntityType entities = 0;
  ++entities;
  MAX_ENTITY = entities;
  return entities;
}
struct AABB{
  glm::vec3 min, max;
  AABB() : min{0.f}, max{0.f} {}
  AABB(const glm::vec3 min, const glm::vec3 max) : min{min}, max{max} {}
  AABB(const AABB& theOther) = default;
  void move(const glm::vec3& direction) { min += direction; max += direction; }
};

bool doesCollidePointvAABB(const glm::vec3& point, const AABB& box);

bool doesCollideAABBvAABB(const AABB& a,const AABB& b);
glm::vec3 getCollisionDirection(const AABB& a,const AABB& b);

struct collision_component {
  AABB box;
  bool isMovable = false;
  bool isColliding = false;
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
};

struct model_system {

  void update(registry& reg) {
    for (std::size_t e = 1; e <= MAX_ENTITY; ++e) {
      if (reg.models.contains(e) && reg.transforms.contains(e)) {
        glm::mat4& modelMatrix  = reg.models.at(e).modelMat;
        modelMatrix             = glm::mat4{1.f};
        auto& modelPosition     = reg.transforms.at(e).pos;
        auto& scale             = reg.transforms.at(e).scale;
        auto& rotationInDegrees = reg.transforms.at(e).rotationInDegrees;
        auto& rotation          = reg.transforms.at(e).rot;
        auto& defaultrotation =
            reg.models.at(e).modelDefaultOrientationRotation;
        modelMatrix = glm::translate(modelMatrix, modelPosition);
        modelMatrix = glm::scale(modelMatrix, scale);
        // rotating the model by the loadoffset
        modelMatrix = modelMatrix * defaultrotation;
        modelMatrix =
            glm::rotate(modelMatrix, glm::radians(rotationInDegrees), rotation);
      }
    }
  }

  void render(registry& reg, Shader& shader) {
    for (std::size_t e = 1; e <= MAX_ENTITY; ++e) {
      if (reg.models.contains(e)) {
        auto& modelMatrix = reg.models.at(e).modelMat;
        auto& model       = reg.models.at(e).model;
        auto& material    = reg.models.at(e).material;
        auto& texture     = reg.models.at(e).texture;
        shader.setMat4fv(modelMatrix, "model");
        shader.use();
        texture.useTexture();
        shader.useMaterial(material, "material.shininess",
                           "material.specularIntensity");
        model.Render();
        shader.unuse();
      }
    }
  }
};

struct transform_system {
  void update(registry& reg, float dt) {
    for (std::size_t e = 1; e <= MAX_ENTITY; ++e) {
      if (reg.transforms.contains(e)) {
        reg.transforms[e].vel += reg.transforms[e].acc * dt;
        //TODO: probably can move isMovable elsewhere and check it first before applying physics
        if (reg.collision_boxes.contains(e) && reg.collision_boxes.at(e).isMovable){
          //1. check for collision
          auto& currentObjectCollisionBox = reg.collision_boxes.at(e).box;
          reg.collision_boxes.at(e).isColliding = false;
          for(auto &[entityID, collider] : reg.collision_boxes){
            // ignore self collision
            if(entityID == e){
              continue;
            }
            auto possibleDisplacement = reg.transforms[e].vel * dt;
            AABB boxAfterMovement(currentObjectCollisionBox);
            boxAfterMovement.move(possibleDisplacement);

            if(doesCollideAABBvAABB(boxAfterMovement,collider.box)){
              reg.collision_boxes.at(e).isColliding = true;
              auto collisionDirection = getCollisionDirection(boxAfterMovement,collider.box);
              auto objVelocity = reg.transforms[e].vel;
              collisionDirection.x *= std::abs(objVelocity.x);
              collisionDirection.y *= std::abs(objVelocity.y);
              collisionDirection.z *= std::abs(objVelocity.z);
              //2. reduce collision velocity component to zero
              reg.transforms[e].vel -= collisionDirection;
            }
          }
          //3. move collision box according to movement
          auto collisionBoxDisplacement = reg.transforms[e].vel * dt;
          currentObjectCollisionBox.move(collisionBoxDisplacement);
        }
        reg.transforms[e].pos += reg.transforms[e].vel * dt;
        reg.transforms[e].rotationInDegrees +=
            reg.transforms[e].rotationvel * dt;
      }
    }
  }
};
