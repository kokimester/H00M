#pragma once

#include <iostream>
#include <stdio.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
private:
  glm::vec3 position;
  glm::vec3 front;
  glm::vec3 up;
  glm::vec3 right;
  glm::vec3 worldUp;

  float yaw;
  float pitch;

  float moveSpeed;
  float turnSpeed;

  bool flashlightOn;

  void update();

public:
  Camera(glm::vec3 startposition, glm::vec3 startup, float startyaw,
         float startpitch, float startMoveSpeed, float startTurnSpeed);
  ~Camera();

  glm::vec3 getCameraPosition() const;
  glm::vec3 getCameraFront() const;
  glm::vec3 getRight() const;

  void invertFlashlight() { flashlightOn = !flashlightOn; }
  bool isFlashlightOn() const { return flashlightOn; }

  glm::mat4 calculateViewMatrix();

  void keyControl(bool *keys, float deltaTime);
  void mouseControl(float xChange, float yChange);
};
