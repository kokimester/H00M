#include "Entity.h"

EntityType MAX_ENTITY = 0;

EntityType create_entity() {
  static EntityType entities = 0;
  ++entities;
  MAX_ENTITY = entities;
  return entities;
}

void model_system::update(registry& reg) {
  for (std::size_t e = 1; e <= MAX_ENTITY; ++e) {
    if (reg.models.contains(e) && reg.transforms.contains(e)) {
      glm::mat4& modelMatrix  = reg.models.at(e).modelMat;
      modelMatrix             = glm::mat4{1.f};
      auto& modelPosition     = reg.transforms.at(e).pos;
      auto& scale             = reg.transforms.at(e).scale;
      auto& rotationInDegrees = reg.transforms.at(e).rotationInDegrees;
      auto& rotation          = reg.transforms.at(e).rot;
      auto& defaultrotation = reg.models.at(e).modelDefaultOrientationRotation;
      modelMatrix           = glm::translate(modelMatrix, modelPosition);
      modelMatrix           = glm::scale(modelMatrix, scale);
      // rotating the model by the loadoffset
      modelMatrix = modelMatrix * defaultrotation;
      modelMatrix =
          glm::rotate(modelMatrix, glm::radians(rotationInDegrees), rotation);
    }
  }
}

void model_system::render(registry& reg, Shader& shader) {
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

void transform_system::update(registry& reg, float dt) {
  for (std::size_t e = 1; e <= MAX_ENTITY; ++e) {
    if (reg.transforms.contains(e)) {
      reg.transforms[e].vel += reg.transforms[e].acc * dt;
      // TODO: probably can move isMovable elsewhere and check it first before
      // applying physics
      if (reg.collision_boxes.contains(e) &&
          reg.collision_boxes.at(e).isMovable) {
        // 1. check for collision
        auto& currentObjectCollisionBox       = reg.collision_boxes.at(e).box;
        reg.collision_boxes.at(e).isColliding = false;
        for (auto& [entityID, collider] : reg.collision_boxes) {
          // ignore self collision
          if (entityID == e) {
            continue;
          }
          auto possibleDisplacement = reg.transforms[e].vel * dt;
          AABB boxAfterMovement(currentObjectCollisionBox);
          boxAfterMovement.move(possibleDisplacement);

          if (doesCollideAABBvAABB(boxAfterMovement, collider.box)) {
            reg.collision_boxes.at(e).isColliding = true;
            auto collisionDirection =
                getCollisionDirection(boxAfterMovement, collider.box);
            auto objVelocity = reg.transforms[e].vel;
            collisionDirection.x *= std::abs(objVelocity.x);
            collisionDirection.y *= std::abs(objVelocity.y);
            collisionDirection.z *= std::abs(objVelocity.z);
            // 2. reduce collision velocity component to zero
            reg.transforms[e].vel -= collisionDirection;
          }
        }
        // 3. move collision box according to movement
        auto collisionBoxDisplacement = reg.transforms[e].vel * dt;
        currentObjectCollisionBox.move(collisionBoxDisplacement);
      }
      reg.transforms[e].pos += reg.transforms[e].vel * dt;
      reg.transforms[e].rotationInDegrees += reg.transforms[e].rotationvel * dt;
    }
  }
}

bool doesCollideAABBvAABB(const AABB& a, const AABB& b) {
  return a.min.x <= b.max.x && a.max.x >= b.min.x && a.min.y <= b.max.y &&
         a.max.y >= b.min.y && a.min.z <= b.max.z && a.max.z >= b.min.z;
}

bool doesCollidePointvAABB(const glm::vec3& point, const AABB& box) {
  return point.x >= box.min.x && point.x <= box.max.x && point.y >= box.min.y &&
         point.y <= box.max.y && point.z >= box.min.z && point.z <= box.max.z;
}

// https://stackoverflow.com/questions/8515198/basic-aabb-collision-using-projection-vector?utm_source=chatgpt.com
glm::vec3 getCollisionDirection(const AABB& a, const AABB& b) {
  // Centers and half-extents
  const glm::vec3 ca = 0.5f * (a.min + a.max);
  const glm::vec3 cb = 0.5f * (b.min + b.max);
  const glm::vec3 ha = 0.5f * (a.max - a.min);
  const glm::vec3 hb = 0.5f * (b.max - b.min);

  const glm::vec3 d = cb - ca; // from A to B

  // Overlap along each axis (sum of half-extents minus center distance)
  float ox = (ha.x + hb.x) - std::abs(d.x);
  float oy = (ha.y + hb.y) - std::abs(d.y);
  float oz = (ha.z + hb.z) - std::abs(d.z);

  // If you want touching to count, clamp tiny negatives to zero
  ox = std::max(0.0f, ox);
  oy = std::max(0.0f, oy);
  oz = std::max(0.0f, oz);

  // if you want collision depth return ox,oy,or oz as well depending on
  // which is the smallest

  auto normal = glm::vec3{0.f};
  // Pick smallest overlap
  if (ox <= oy && ox <= oz) {
    normal = glm::vec3((d.x >= 0.0f) ? 1.0f : -1.0f, 0.0f, 0.0f);
  } else if (oy <= oz) {
    normal = glm::vec3(0.0f, (d.y >= 0.0f) ? 1.0f : -1.0f, 0.0f);
  } else {
    normal = glm::vec3(0.0f, 0.0f, (d.z >= 0.0f) ? 1.0f : -1.0f);
  }
  return normal;
}
