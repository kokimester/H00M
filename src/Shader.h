#pragma once
#include <glew/glew.h>
#include <string>
#include <iostream>
#include <fstream>

#include <glew/glew.h>
#include <GLFW\glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DirectionalLight.h"
#include "PointLight.h"
#include "Spotlight.h"
#include "Material.h"
#include "CommonValues.h"

class Shader
{
private:
	GLuint id;
	std::string loadShaderSource(const char* fileName);
	GLuint addShader(const char* shaderCode, GLenum shaderType);
	
	int pointLightCount;
	int spotLightCount;

	struct {
		GLuint uniformColour;
		GLuint uniformAmbientIntensity;
		GLuint uniformDiffuseIntensity;
	
		GLuint uniformPosition;
		GLuint uniformConstant;
		GLuint uniformLinear;
		GLuint uniformExponent;
	} uniformPointLight[MAX_POINT_LIGHTS];

	struct {
		GLuint uniformColour;
		GLuint uniformAmbientIntensity;
		GLuint uniformDiffuseIntensity;

		GLuint uniformPosition;
		GLuint uniformConstant;
		GLuint uniformLinear;
		GLuint uniformExponent;

		GLuint uniformDirection;
		GLuint uniformEdge;
	}uniformSpotLight[MAX_SPOT_LIGHTS];

	void linkProgram(GLuint vertexShader, GLuint fragmentShader);

public:
	Shader(const char* vertexFile, const char* fragmentFile);
	~Shader() { glDeleteProgram(id); }

	void use();
	void unuse();

	void clearShader();

	void setSpotLights(Spotlight* arrayToSet, unsigned int spotLightCount);
	void setPointLights(PointLight* arrayToSet, unsigned int lightCount);
	void setDirectionalLight(DirectionalLight& toSet);

	void useLight(const DirectionalLight& toUse, const GLchar* colourName, const GLchar* ambientName,
		const GLchar* directionName, const GLchar* diffuseName);
	void useMaterial(const Material& toUse, const GLchar* shininessName, const GLchar* specularName);

	void set1i(GLint value, const GLchar* name);
	void set1f(GLfloat value, const GLchar* name);
	void setVec2f(glm::fvec2 value, const GLchar* name);
	void setVec3f(glm::fvec3 value, const GLchar* name);
	void setVec4f(glm::fvec4 value, const GLchar* name);
	void setMat3fv(glm::mat3 value, const GLchar* name, GLboolean transpose = GL_FALSE);
	void setMat4fv(glm::mat4 value, const GLchar* name, GLboolean transpose = GL_FALSE);
};

