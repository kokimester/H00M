#pragma once

#include <cassert>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Mesh.h"
#include "Texture.h"

//===================
//		EXCEPTIONS
//		std::invalid_argument <- FAILED TO LOAD MODEL
//
//===================
// TODO: separate Model into NonSkinned and Skinned classes to reduce memory
// footprint on unanimated objects.
class Model {
public:
  enum class ANIMATION_STATE : uint8_t {
    //for models without any animations
    NO_ANIMATION,
    //for models with animations currently not animating
    IDLE,
    IN_ANIMATION
  };
  // TODO: remove this baked in path
  Model() : missingTexture{"../textures/missing.jpg"} {
    missingTexture.loadTexture();
  }
  ~Model();
  bool LoadMesh(const std::filesystem::path& path);
  void Render();

  size_t NumBones() const { return m_BoneNameToIndexMap.size(); }
  const std::string& GetActiveAnimationName() const;
  void SetActiveAnimation(const std::string& animation);
  void Animate(GLfloat deltaTime);
  void GetBoneTransforms(std::vector<glm::mat4>& Transforms);
  ANIMATION_STATE GetAnimationState() const;
  float GetAnimationProgress() const;

private:
#define MAX_NUM_BONES_PER_VERTEX 4

  void Clear();

  bool InitFromScene();
  void CountVerticesAndIndices(unsigned int& NumVertices,
                               unsigned int& NumIndices);
  void ReserveSpace(unsigned int NumVertices, unsigned int NumIndices);
  void InitAllMeshes();
  void InitSingleMesh(size_t MeshIndex, const aiMesh* paiMesh);
  void InitMaterials();
  void PopulateBuffers();

  struct VertexBoneData {
    uint32_t BoneIDs[MAX_NUM_BONES_PER_VERTEX] = {0};
    float Weights[MAX_NUM_BONES_PER_VERTEX] = {0.0f};

    VertexBoneData() {}

    void AddBoneData(size_t BoneID, float Weight) {
      for (size_t i = 0; i < MAX_NUM_BONES_PER_VERTEX; i++) {
        if (Weights[i] < 0.0001) {
          BoneIDs[i] = BoneID;
          Weights[i] = Weight;
          return;
        }
      }
      // should never get here - more bones than we have space for
      assert(0);
    }
  };

  void LoadMeshBones(size_t MeshIndex, const aiMesh* paiMesh);
  void LoadSingleBone(size_t MeshIndex, const aiBone* pBone);
  size_t GetBoneId(const aiBone* pBone);

  void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime,
                               const aiNodeAnim* pNodeAnim);
  void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime,
                                const aiNodeAnim* pNodeAnim);
  void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime,
                                const aiNodeAnim* pNodeAnim);
  size_t FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
  size_t FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
  size_t FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
  const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation,
                                 const std::string& NodeName);
  void ReadNodeHierarchy(float AnimationTime, const aiNode* pNode,
                         const glm::mat4& ParentTransform);
#define INVALID_MATERIAL 0xFFFFFFFF

  enum BUFFER_TYPE {
    INDEX_BUFFER = 0,
    POS_VB = 1,
    TEXCOORD_VB = 2,
    NORMAL_VB = 3,
    BONE_VB = 4,
    NUM_BUFFERS = 5
  };

  GLuint m_VAO = 0;
  GLuint m_Buffers[NUM_BUFFERS] = {0};

  struct BasicMeshEntry {
    BasicMeshEntry() {
      NumIndices = 0;
      BaseVertex = 0;
      BaseIndex = 0;
      MaterialIndex = INVALID_MATERIAL;
    }

    unsigned int NumIndices;
    unsigned int BaseVertex;
    unsigned int BaseIndex;
    unsigned int MaterialIndex;
  };

  Assimp::Importer m_Importer;
  const aiScene* m_Scene = nullptr;

  std::vector<BasicMeshEntry> m_Meshes;
  std::vector<std::unique_ptr<Texture>> m_TextureList;
  Texture missingTexture;

  // Temporary space for vertex stuff before we load them into the GPU
  std::vector<glm::vec3> m_Positions;
  std::vector<glm::vec3> m_Normals;
  std::vector<glm::vec2> m_TexCoords;
  std::vector<unsigned int> m_Indices;
  std::vector<VertexBoneData> m_Bones;

  std::map<std::string, size_t> m_BoneNameToIndexMap;

  // --- Animations ---

  size_t m_ActiveAnimation = 0;
  std::map<std::string, size_t> m_AnimationNameToIndexMap;
  float m_AnimationTime = 0.f;
  //TODO: figure out a better way to store this 
  //(maybe a struct for all animations with all relevant information stored insed)
  float m_TicksPerSecond = 0.f;
  
  ANIMATION_STATE m_AnimationState = ANIMATION_STATE::NO_ANIMATION;

  struct BoneInfo {
    glm::mat4 OffsetMatrix;
    glm::mat4 FinalTransformation;

    BoneInfo(const glm::mat4& Offset) {
      OffsetMatrix = glm::transpose(Offset);
      FinalTransformation = glm::mat4{0.f};
    }
  };

  std::vector<BoneInfo> m_BoneInfo;
  glm::mat4 m_GlobalInverseTransform;
};
