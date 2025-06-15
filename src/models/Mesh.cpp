#include "Mesh.h"

Mesh::Mesh() : VAO(0), VBO(0), IBO(0), indexCount(0) {
  /* std::cout << "Mesh default construction" << std::endl; */
}

Mesh::~Mesh() {
  /* std::cout << "Mesh destructor! (" << VAO << ")" << std::endl; */
  clearMesh();
}

Mesh::Mesh(const Mesh &&theOther)
    : VAO{theOther.VAO}, VBO{theOther.VBO}, IBO{theOther.IBO},
      indexCount{theOther.indexCount} {
  /* std::cout << "Mesh move construction (" << VAO << ")" << std::endl; */
}

void Mesh::createMesh(float *vertices, unsigned int *indices, int numOfVertices,
                      int numOfIndices) {
  /* std::cout << "creating mesh with " << numOfVertices << " vertices and " */
  /*           << numOfIndices << " indices\n"; */
  indexCount = numOfIndices;

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO); // BIND

  glGenBuffers(1, &IBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * numOfIndices,
               indices, GL_STATIC_DRAW);

  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO); // BIND
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * numOfVertices, vertices,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 8, 0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 8,
                        (void *)(sizeof(vertices[0]) * 3));
  glEnableVertexAttribArray(1);
  // melyik channelen, mennyi adat					sor
  // hossza						offset
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 8,
                        (void *)(sizeof(vertices[0]) * 5));
  glEnableVertexAttribArray(2); // shaderbe melyik channelen van

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
