#pragma once

#include <exception>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <filesystem>
#include <memory>
#include <map>
#include <cstdint>
#include <cassert>

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
//TODO: separate Model into NonSkinned and Skinned classes to reduce memory footprint on unanimated objects.
class Model {
public:
  Model() : missingTexture{"../textures/missing.jpg"} {
    missingTexture.loadTexture();
  }
  ~Model();
  bool LoadMesh(const std::filesystem::path& path);
  void Render();

  size_t NumBones() const
  {
      return (size_t)m_BoneNameToIndexMap.size();
  }

  size_t GetActiveAnimationIndex() const {
    return m_ActiveAnimation;
  }

  void SetActiveAnimation(size_t index) {
    assert(index < m_Scene->mNumAnimations);
    m_ActiveAnimation = index;
  }
  
  void GetBoneTransforms(float AnimationTimeSec, std::vector<glm::mat4>& Transforms);
private:

  #define MAX_NUM_BONES_PER_VERTEX 4

    void Clear();

    bool InitFromScene();
    void CountVerticesAndIndices(unsigned int& NumVertices, unsigned int& NumIndices);
    void ReserveSpace(unsigned int NumVertices, unsigned int NumIndices);
    void InitAllMeshes();
    void InitSingleMesh(size_t MeshIndex, const aiMesh* paiMesh);
    void InitMaterials();
    void PopulateBuffers();

    struct VertexBoneData
    {
        uint32_t BoneIDs[MAX_NUM_BONES_PER_VERTEX] = { 0 };
        float Weights[MAX_NUM_BONES_PER_VERTEX] = { 0.0f };

        VertexBoneData()
        {
        }

        void AddBoneData(size_t BoneID, float Weight)
        {
            for (size_t i = 0 ; i < MAX_NUM_BONES_PER_VERTEX ; i++) {
                if (Weights[i] < 0.0001) {
                    BoneIDs[i] = BoneID;
                    Weights[i] = Weight;
                    //printf("Adding bone %d weight %f at index %i\n", BoneID, Weight, i);
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

    void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    size_t FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
    size_t FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
    size_t FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
    const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const std::string& NodeName);
    void ReadNodeHierarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform);
#define INVALID_MATERIAL 0xFFFFFFFF

    enum BUFFER_TYPE {
        INDEX_BUFFER = 0,
        POS_VB       = 1,
        TEXCOORD_VB  = 2,
        NORMAL_VB    = 3,
        BONE_VB      = 4,
        NUM_BUFFERS  = 5
    };

    GLuint m_VAO = 0;
    GLuint m_Buffers[NUM_BUFFERS] = { 0 };

    struct BasicMeshEntry {
        BasicMeshEntry()
        {
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

    std::map<std::string,size_t> m_BoneNameToIndexMap;

    //animations
    size_t m_ActiveAnimation = 0;

    struct BoneInfo
        {
            glm::mat4 OffsetMatrix;
            glm::mat4 FinalTransformation;

            BoneInfo(const glm::mat4& Offset)
            {
                OffsetMatrix = glm::transpose(Offset);
                FinalTransformation = glm::mat4{0.f};
            }
        };

        std::vector<BoneInfo> m_BoneInfo;
        glm::mat4 m_GlobalInverseTransform;
};
