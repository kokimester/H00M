#pragma once

#include "CommonValues.h"
#include <print>
#include <vector>

/* #include <GLES3/gl3.h> */
/* #include <GLFW/glfw3.h> */
#include <glad/glad.h>

class Mesh {
private:
  unsigned int VAO, VBO, IBO;
  unsigned int indexCount;

public:
  Mesh();
  ~Mesh();
  Mesh(const Mesh &&);
  Mesh(const Mesh &) = delete;
  void createMesh(const std::vector<GLfloat> &vertices,
                  const std::vector<unsigned int> &indices);
  void renderMesh();
  void clearMesh();
};
