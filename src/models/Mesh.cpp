#include "Mesh.h"
#include <cassert>

void Mesh::VertexBoneData::addBoneData(unsigned int BoneID, float Weight){
    for (size_t i = 0 ; i < NUM_BONES_PER_VERTEX; i++) {
        if (Weights[i] == 0.0) {
            IDs[i] = BoneID;
            Weights[i] = Weight;
            return;
        }
    }
    // should never get here - more bones than we have space for
    std::println("Error in addBoneData! More bones were added than we have space for!");
    assert(0);
}

Mesh::Mesh() : VAO(0), VBO(0), IBO(0),m_BoneBufferID(0), indexCount(0) {
  /* std::cout << "Mesh default construction" << std::endl; */
}

Mesh::~Mesh() {
  /* std::cout << "Mesh destructor! (" << VAO << ")" << std::endl; */
  printf("Mesh is destructed %p: %d\n",(void*)this, VAO);
  clearMesh();
}

Mesh::Mesh(const Mesh &&theOther)
    : VAO{theOther.VAO}, VBO{theOther.VBO}, IBO{theOther.IBO}, m_BoneBufferID{theOther.m_BoneBufferID},
      indexCount{theOther.indexCount} {
  /* std::cout << "Mesh move construction (" << VAO << ")" << std::endl; */
  printf("Mesh is move constructed %p: %d\n",(void*)this, VAO);
}

void Mesh::createMesh(const std::vector<GLfloat>& vertices, const std::vector<unsigned int>& indices, const std::vector<VertexBoneData>& bones) {
  indexCount = indices.size();
  
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO); // BIND
  printf("createMesh this ptr: %p, VAO: %d\n",(void*)this, VAO);

  GLuint VERTEX_LOCATION      = 0;
  GLuint TEXTURE_LOCATION     = 1;
  GLuint NORMAL_LOCATION      = 2;
  GLuint BONE_ID_LOCATION     = 3;
  GLuint BONE_WEIGHT_LOCATION = 4;

  glGenBuffers(1, &IBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);

  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

  
  //TODO: convert the first parameters to ENUMS/DEFINES
  glEnableVertexAttribArray(VERTEX_LOCATION);
  glVertexAttribPointer(VERTEX_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 8, 0);
  
  glEnableVertexAttribArray(TEXTURE_LOCATION);
  glVertexAttribPointer(TEXTURE_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 8, (void *)(sizeof(vertices[0]) * 3));
  
  glEnableVertexAttribArray(NORMAL_LOCATION);
  glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 8, (void *)(sizeof(vertices[0]) * 5));
  
  glGenBuffers(1, &m_BoneBufferID);
  glBindBuffer(GL_ARRAY_BUFFER, m_BoneBufferID);
  glBufferData(GL_ARRAY_BUFFER, sizeof(bones[0]) * bones.size(), &bones[0], GL_STATIC_DRAW);
  
  glEnableVertexAttribArray(BONE_ID_LOCATION);
  glVertexAttribIPointer(BONE_ID_LOCATION, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);

  glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
  glVertexAttribPointer(BONE_WEIGHT_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)(4 * sizeof(int32_t)));

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
