#pragma once

#include <iostream>
#include <stdio.h>

#include <glew/glew.h>
#include <GLFW\glfw3.h>

class Mesh
{
private:
	GLuint VAO, VBO, IBO;
	unsigned int indexCount;
public:
	Mesh();
	~Mesh();
	void createMesh(GLfloat* vertices, unsigned int *indices, int numOfVertices, int numOfIndices);
	void renderMesh();
	void clearMesh();
};

