#pragma once

#include <iostream>
#include <stdio.h>

#include <glew/glew.h>
#include <GLFW\glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
private:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	GLfloat yaw;
	GLfloat pitch;

	GLfloat moveSpeed;
	GLfloat turnSpeed;

	GLboolean flashlightOn;

	void update();

public:
	Camera(glm::vec3 startposition, glm::vec3 startup, GLfloat startyaw, GLfloat startpitch, GLfloat startMoveSpeed, GLfloat startTurnSpeed);
	~Camera();

	glm::vec3 getCameraPosition() const;
	glm::vec3 getCameraFront() const;
	glm::vec3 getRight() const;

	void invertFlashlight() { flashlightOn = !flashlightOn; }
	bool isFlashlightOn() const { return flashlightOn; }

	glm::mat4 calculateViewMatrix();

	void keyControl(bool* keys, GLfloat deltaTime);
	void mouseControl(GLfloat xChange, GLfloat yChange);
};

