#include "Player.h"

Player::Player(EntityType entityID, registry& reg, Camera& camera)
    : m_EntityID{entityID}, m_Registry{reg}, m_PlayerCamera{camera} {}

void Player::handleMouseInput(float xChange, float yChange) {
  m_PlayerCamera.mouseControl(xChange, yChange);
}

void Player::handleKeyboardInput(bool* keys, float deltaTime) {
  float moveSpeedModifier = 200.f;
  float speed             = moveSpeedModifier * deltaTime;
  auto front              = m_PlayerCamera.getCameraFront();
  auto right              = m_PlayerCamera.getRight();
  front                   = {front.x, 0.f, front.z};
  right                   = {right.x, 0.f, right.z};
  if (std::abs(front.x) > 0.001f || std::abs(front.y) > 0.001f) {
    front = glm::normalize(front);
  }
  if (std::abs(right.x) > 0.001f || std::abs(right.y) > 0.001f) {
    right = glm::normalize(right);
  }
  auto velocity = glm::vec3{0.f};
  if (keys[GLFW_KEY_LEFT_SHIFT]) {
    speed *= 2;
  }
  if (keys[GLFW_KEY_W]) {
    velocity += front * speed;
  }
  if (keys[GLFW_KEY_S]) {
    velocity -= front * speed;
  }
  if (keys[GLFW_KEY_A]) {
    velocity -= right * speed;
  }
  if (keys[GLFW_KEY_D]) {
    velocity += right * speed;
  }
  if (keys[GLFW_KEY_SPACE]) {
    velocity += m_PlayerCamera.getWorldUp() * speed / 3.f;
  }
  /* if (keys[GLFW_KEY_LEFT_CONTROL]) { */
  /*   velocity -= worldUp * speed; */
  /* } */
  /* if (keys[GLFW_KEY_F]) { */
  /*   invertFlashlight(); */
  /*   keys[GLFW_KEY_F] = false; */
  /* } */
  m_Registry.transforms[m_EntityID].vel.x = velocity.x;
  m_Registry.transforms[m_EntityID].vel.y += velocity.y;
  m_Registry.transforms[m_EntityID].vel.z = velocity.z;
}

void Player::update(float deltaTime) {
  auto playerPosition = m_Registry.transforms[m_EntityID].pos;
  auto cameraOffset   = glm::vec3{0.f, 1.f, 0.0f};
  m_PlayerCamera.setPosition(playerPosition + cameraOffset);
}
