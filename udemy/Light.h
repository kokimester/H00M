#pragma once

#include <glew/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Light
{
protected:
	glm::vec3 colour;
	GLfloat ambientIntensity;
	GLfloat diffuseIntensity;

public:
	Light();
	Light(GLfloat red, GLfloat green, GLfloat blue, GLfloat aIntensity,
		GLfloat dIntensity);
	~Light();

	glm::vec3 getColour() const { return colour; }
	GLfloat getAmbientIntensity() const { return ambientIntensity; }
	GLfloat getDiffuseIntensity() const { return diffuseIntensity; }

};

