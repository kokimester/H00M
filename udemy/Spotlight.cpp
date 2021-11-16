#include "Spotlight.h"

Spotlight::Spotlight()
: PointLight()
{
	direction = glm::vec3(0.0f, -1.0f, 0.0f);
	edge = 0.0f;
	procEdge = cosf(glm::radians(edge));
}

Spotlight::Spotlight(GLfloat red, GLfloat green, GLfloat blue, GLfloat aIntensity, GLfloat dIntensity,
	GLfloat xPos, GLfloat yPos, GLfloat zPos,
	GLfloat xDir, GLfloat yDir, GLfloat zDir,
	GLfloat pConst, GLfloat pLinear, GLfloat pExp,
	GLfloat pEdge)
	: PointLight(red,green,blue,aIntensity,dIntensity,xPos,yPos,zPos,pConst,pLinear,pExp)
{
	direction = glm::normalize(glm::vec3(xDir, yDir, zDir));
	edge = pEdge;
	procEdge = cosf(glm::radians(edge));
}

void Spotlight::useLight(GLuint ambientIntensityLocation, GLuint ambientColourLocation, GLuint diffuseIntensityLocation,
						GLuint positionLocation, GLuint directionLocation,
						GLuint constantLocation, GLuint linearLocation, GLuint exponentLocation,
						GLuint edgeLocation)
{
	glUniform3f(ambientColourLocation, colour.x, colour.y, colour.z);
	glUniform1f(ambientIntensityLocation, ambientIntensity);
	glUniform1f(diffuseIntensityLocation, diffuseIntensity);

	glUniform3f(positionLocation, position.x, position.y, position.z);

	glUniform1f(constantLocation, constant);
	glUniform1f(linearLocation, linear);
	glUniform1f(exponentLocation, exponent);

	glUniform3f(directionLocation, direction.x, direction.y, direction.z);
	glUniform1f(edgeLocation, procEdge);
}
