#include "Mesh.h"

Mesh::Mesh() : VAO(0), VBO(0), IBO(0), indexCount(0) {
  /* std::cout << "Mesh default construction" << std::endl; */
}

Mesh::~Mesh() {
  /* std::cout << "Mesh destructor! (" << VAO << ")" << std::endl; */
  /* printf("Mesh is destructed %p: %d\n", (void *)this, VAO); */
  clearMesh();
}

Mesh::Mesh(const Mesh &&theOther)
    : VAO{theOther.VAO}, VBO{theOther.VBO}, IBO{theOther.IBO},
      indexCount{theOther.indexCount} {
  /* std::cout << "Mesh move construction (" << VAO << ")" << std::endl; */
  /* printf("Mesh is move constructed %p: %d\n", (void *)this, VAO); */
}

void Mesh::createMesh(const std::vector<GLfloat> &vertices,
                      const std::vector<unsigned int> &indices) {
  indexCount = indices.size();

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO); // BIND
  /* printf("createMesh this ptr: %p, VAO: %d\n", (void *)this, VAO); */

  GLuint VERTEX_LOCATION = 0;
  GLuint TEXTURE_LOCATION = 1;
  GLuint NORMAL_LOCATION = 2;

  glGenBuffers(1, &IBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(),
               &indices[0], GL_STATIC_DRAW);

  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(),
               &vertices[0], GL_STATIC_DRAW);

  // TODO: convert the first parameters to ENUMS/DEFINES
  glEnableVertexAttribArray(VERTEX_LOCATION);
  glVertexAttribPointer(VERTEX_LOCATION, 3, GL_FLOAT, GL_FALSE,
                        sizeof(vertices[0]) * 8, 0);

  glEnableVertexAttribArray(TEXTURE_LOCATION);
  glVertexAttribPointer(TEXTURE_LOCATION, 2, GL_FLOAT, GL_FALSE,
                        sizeof(vertices[0]) * 8,
                        (void *)(sizeof(vertices[0]) * 3));

  glEnableVertexAttribArray(NORMAL_LOCATION);
  glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE,
                        sizeof(vertices[0]) * 8,
                        (void *)(sizeof(vertices[0]) * 5));

  glBindBuffer(GL_ARRAY_BUFFER, 0); // UNBIND
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glBindVertexArray(0); // UNBIND
}

void Mesh::renderMesh() {
  glBindVertexArray(VAO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
  glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void Mesh::clearMesh() {
  /* VAO = 0; */
  /* VBO = 0; */
  /* IBO = 0; */
  /* indexCount = 0; */
  /* glBindVertexArray(0); // UNBIND */
}
