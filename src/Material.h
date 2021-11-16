#pragma once

#include <glew/glew.h>

class Material
{
private:
	GLfloat specularIntensity;
	GLfloat shininess;
public:
	Material();
	Material(GLfloat sIntensity, GLfloat shine);
	~Material() {};

	GLfloat getShininess() const { return shininess; }
	GLfloat getSpecularIntensity() const { return specularIntensity; }

	void useMaterial(GLuint specularIntensityLocation, GLuint shininessLocation) {
		glUniform1f(specularIntensityLocation, specularIntensity);
		glUniform1f(shininessLocation, shininess);
	}
};

