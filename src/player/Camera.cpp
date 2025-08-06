#include "Camera.h"

Camera::Camera(glm::vec3 startposition, glm::vec3 startup, float startyaw,
               float startpitch, float startMoveSpeed, float startTurnSpeed) {
  position = startposition;
  worldUp  = startup;
  yaw      = startyaw;
  pitch    = startpitch;
  front    = glm::vec3(0.0f, 0.0f, -1.0f);

  moveSpeed = startMoveSpeed;
  turnSpeed = startTurnSpeed;

  flashlightOn = false;

  update();
}

Camera::~Camera() {}

glm::vec3 Camera::getCameraPosition() const { return position; }

glm::vec3 Camera::getCameraFront() const { return front; }

glm::vec3 Camera::getRight() const { return right; }

glm::vec3 Camera::getWorldUp() const { return worldUp; }

glm::mat4 Camera::calculateViewMatrix() const {
  return glm::lookAt(position, position + front, up);
}

void Camera::setPosition(const glm::vec3& p_position) { position = p_position; }

void Camera::keyControl(bool* keys, float deltaTime) {}

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
  front   = glm::normalize(front);

  right = glm::normalize(glm::cross(front, worldUp));
  up    = glm::normalize(glm::cross(right, front));
}
