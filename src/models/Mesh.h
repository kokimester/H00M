#pragma once

#include "CommonValues.h"
#include <vector>
#include <print>

/* #include <GLES3/gl3.h> */
/* #include <GLFW/glfw3.h> */
#include <glad/glad.h>


class Mesh {
private:
  unsigned int VAO, VBO, IBO,m_BoneBufferID;
  unsigned int indexCount;

public:
  static constexpr unsigned int NUM_BONES_PER_VERTEX = 4;
  struct VertexBoneData
  {
      unsigned int IDs[NUM_BONES_PER_VERTEX];
      GLfloat Weights[NUM_BONES_PER_VERTEX];
      VertexBoneData(){
        for(size_t i = 0; i < NUM_BONES_PER_VERTEX; ++i){
          IDs[i] = 0;
          Weights[i] = 0.f;
        }
      };
      void addBoneData(unsigned int BoneID, float Weight);
  };
  Mesh();
  ~Mesh();
  Mesh(const Mesh &&);
  Mesh(const Mesh &) = delete;
  void createMesh(const std::vector<GLfloat>& vertices,const std::vector<unsigned int>& indices, const std::vector<VertexBoneData>& bones);
  void renderMesh();
  void clearMesh();
};
