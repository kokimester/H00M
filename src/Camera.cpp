#include "Camera.h"

Camera::Camera(glm::vec3 startposition, glm::vec3 startup, float startyaw,
               float startpitch, float startMoveSpeed, float startTurnSpeed) {
  position = startposition;
  worldUp = startup;
  yaw = startyaw;
  pitch = startpitch;
  front = glm::vec3(0.0f, 0.0f, -1.0f);

  moveSpeed = startMoveSpeed;
  turnSpeed = startTurnSpeed;

  flashlightOn = false;

  update();
}

Camera::~Camera() {}

glm::vec3 Camera::getCameraPosition() const { return position; }

glm::vec3 Camera::getCameraFront() const { return front; }

glm::vec3 Camera::getRight() const { return right; }

glm::mat4 Camera::calculateViewMatrix() {
  return glm::lookAt(position, position + front, up);
}

void Camera::keyControl(bool *keys, float deltaTime) {
  float velocity = moveSpeed * deltaTime;
  bool prevF = false;

  if (keys[GLFW_KEY_W]) {
    position += front * velocity;
  }
  if (keys[GLFW_KEY_S]) {
    position -= front * velocity;
  }
  if (keys[GLFW_KEY_A]) {
    position -= right * velocity;
  }
  if (keys[GLFW_KEY_D]) {
    position += right * velocity;
  }
  if (keys[GLFW_KEY_SPACE]) {
    position += worldUp * velocity;
  }
  if (keys[GLFW_KEY_LEFT_CONTROL]) {
    position -= worldUp * velocity;
  }
  if (keys[GLFW_KEY_F]) {
    invertFlashlight();
    keys[GLFW_KEY_F] = false;
  }
}

void Camera::mouseControl(float xChange, float yChange) {
  yaw += turnSpeed * xChange;
  pitch += turnSpeed * yChange;
  if (pitch > 89.0f)
    pitch = 89.0f;
  if (pitch < -89.0f)
    pitch = -89.0f;
  update();
}

void Camera::update() {
  front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  front.y = sin(glm::radians(pitch));
  front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  front = glm::normalize(front);

  right = glm::normalize(glm::cross(front, worldUp));
  up = glm::normalize(glm::cross(right, front));
}
