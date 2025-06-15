#pragma once

#include <iostream>
#include <stdio.h>

/* #include <GLES3/gl3.h> */
/* #include <GLFW/glfw3.h> */
#include <glad/glad.h>

class Mesh {
private:
  uint VAO, VBO, IBO;
  unsigned int indexCount;

public:
  Mesh();
  ~Mesh();
  Mesh(const Mesh &&);
  Mesh(const Mesh &) = delete;
  void createMesh(float *vertices, unsigned int *indices, int numOfVertices,
                  int numOfIndices);
  void renderMesh();
  void clearMesh();
};
