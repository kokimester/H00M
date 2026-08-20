#include "Model.h"
#include "Utility.h"
#include <print>

void Model::InitMaterials() {
  m_TextureList.resize(m_Scene->mNumMaterials);
  // std::println("Material count: {}", m_Scene->mNumMaterials);

  for (size_t i = 0; i < m_Scene->mNumMaterials; i++) {
    aiMaterial* material = m_Scene->mMaterials[i];

    // std::println("[{}] Material name: {}",i,material->GetName().data);
    m_TextureList[i] = nullptr;
    int textureCount = material->GetTextureCount(aiTextureType_DIFFUSE);
    std::println("Material name: {}", material->GetName().C_Str());
    std::println("Found {} textures.", textureCount);
    if (textureCount) {
      aiString path;
      if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) {
        int idx = std::string(path.data).rfind('\\');
        std::println("Texture path: {}", path.data);
        std::string fileName = std::string(path.data).substr(idx + 1);

        //TODO: remove this obnoxious hack and move it to something global
        namespace fs = std::filesystem;
        auto projectPath = fs::current_path();
        if (!isValidProjectPath(projectPath)) {
          std::println(stderr, "Error occured while validating project path!");
          m_TextureList[i].reset();
        }
        auto textureDir("textures");
        std::string texPath = fs::path(projectPath / textureDir / fileName).string();

        m_TextureList[i] = std::make_unique<Texture>(texPath.c_str());
        if (!m_TextureList[i]->loadTexture()) {
          std::println("Failed to load texture at: ", texPath);
          m_TextureList[i].reset();
        }
      }
    }
    // TODO: remove this
    if (!m_TextureList[i]) {
      std::println("Loading missing texture...");
      m_TextureList[i] = std::make_unique<Texture>("../textures/missing.jpg");
      m_TextureList[i]->loadTexture();
    }
  }
  missingTexture.loadTexture();
}

#define POSITION_LOCATION 0
#define TEX_COORD_LOCATION 1
#define NORMAL_LOCATION 2
#define BONE_ID_LOCATION 3
#define BONE_WEIGHT_LOCATION 4

Model::~Model() { Clear(); }

void Model::Clear() {
  if (m_Buffers[0] != 0) {
    // TODO: fix this NUM_BUFFERS nonsense
    glDeleteBuffers(NUM_BUFFERS, m_Buffers);
  }

  if (m_VAO != 0) {
    glDeleteVertexArrays(1, &m_VAO);
    m_VAO = 0;
  }
}

bool Model::LoadMesh(const std::filesystem::path& path) {
  auto Filename = path.string();
  // Release the previously loaded mesh (if it exists)
  Clear();

  // Create the VAO
  glGenVertexArrays(1, &m_VAO);
  glBindVertexArray(m_VAO);

  // Create the buffers for the vertices attributes
  // TODO: fix this NUM_BUFFERS nonsense
  glGenBuffers(NUM_BUFFERS, m_Buffers);

  bool initSuccess   = false;
  unsigned int flags = aiProcess_Triangulate | aiProcess_FlipUVs;
  flags |= aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices;

  std::println("----Loading mesh {}", Filename);
  double startTime = 0;
  startTime        = glfwGetTime();
  m_Scene          = m_Importer.ReadFile(Filename.c_str(), flags);
  std::println("----Loaded file in {} ms",
               (glfwGetTime() - startTime) * 1000.f);

  if (m_Scene) {
    m_GlobalInverseTransform =
        convertAssimpMatToGLM(m_Scene->mRootNode->mTransformation);
    m_GlobalInverseTransform = glm::inverse(m_GlobalInverseTransform);
    std::println("----Loading scene: {}", m_Scene->mName.C_Str());
    startTime   = glfwGetTime();
    initSuccess = InitFromScene();
    std::println("----Loaded scene in {} ms",
                 (glfwGetTime() - startTime) * 1000.f);
  } else {
    printf("Error parsing '%s': '%s'\n", Filename.c_str(),
           m_Importer.GetErrorString());
  }

  // Make sure the VAO is not changed from the outside
  glBindVertexArray(0);

  return initSuccess;
}

bool Model::InitFromScene() {
  m_Meshes.resize(m_Scene->mNumMeshes);
  m_TextureList.resize(m_Scene->mNumMaterials);

  unsigned int NumVertices = 0;
  unsigned int NumIndices  = 0;

  CountVerticesAndIndices(NumVertices, NumIndices);

  ReserveSpace(NumVertices, NumIndices);

  InitAllMeshes();

  InitMaterials();

  PopulateBuffers();

  for (size_t i = 0; i < m_Scene->mNumAnimations; ++i) {
    std::string animatioName = m_Scene->mAnimations[i]->mName.C_Str();
    std::println("{}: {}", i, animatioName);
    m_AnimationNameToIndexMap[animatioName] = i;
  }

  return glGetError() == GL_NO_ERROR;
}

void Model::CountVerticesAndIndices(unsigned int& NumVertices,
                                    unsigned int& NumIndices) {
  for (unsigned int i = 0; i < m_Meshes.size(); i++) {
    m_Meshes[i].MaterialIndex = m_Scene->mMeshes[i]->mMaterialIndex;
    m_Meshes[i].NumIndices    = m_Scene->mMeshes[i]->mNumFaces * 3;
    m_Meshes[i].BaseVertex    = NumVertices;
    m_Meshes[i].BaseIndex     = NumIndices;

    NumVertices += m_Scene->mMeshes[i]->mNumVertices;
    NumIndices += m_Meshes[i].NumIndices;
  }
}

void Model::ReserveSpace(unsigned int NumVertices, unsigned int NumIndices) {
  m_Positions.reserve(NumVertices);
  m_Normals.reserve(NumVertices);
  m_TexCoords.reserve(NumVertices);
  m_Indices.reserve(NumIndices);
  m_Bones.resize(NumVertices);
}

void Model::InitAllMeshes() {
  for (unsigned int i = 0; i < m_Meshes.size(); i++) {
    const aiMesh* paiMesh = m_Scene->mMeshes[i];
    InitSingleMesh(i, paiMesh);
  }
}

void Model::InitSingleMesh(size_t MeshIndex, const aiMesh* paiMesh) {
  const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

  // Populate the vertex attribute vectors
  for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {

    const aiVector3D& pPos = paiMesh->mVertices[i];
    m_Positions.push_back(glm::vec3(pPos.x, pPos.y, pPos.z));

    if (paiMesh->mNormals) {
      const aiVector3D& pNormal = paiMesh->mNormals[i];
      // TODO: for some reason normals are backwards with this import strategy,
      // so we have to flip it here
      m_Normals.push_back(-glm::vec3(pNormal.x, pNormal.y, pNormal.z));
    } else {
      aiVector3D Normal(0.0f, 1.0f, 0.0f);
      m_Normals.push_back(glm::vec3(Normal.x, Normal.y, Normal.z));
    }

    const aiVector3D& pTexCoord =
        paiMesh->HasTextureCoords(0) ? paiMesh->mTextureCoords[0][i] : Zero3D;
    m_TexCoords.push_back(glm::vec2(pTexCoord.x, pTexCoord.y));
  }

  LoadMeshBones(MeshIndex, paiMesh);

  // Populate the index buffer
  for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
    const aiFace& Face = paiMesh->mFaces[i];
    //        printf("num indices %d\n", Face.mNumIndices);
    //        assert(Face.mNumIndices == 3);
    m_Indices.push_back(Face.mIndices[0]);
    m_Indices.push_back(Face.mIndices[1]);
    m_Indices.push_back(Face.mIndices[2]);
  }
}

void Model::LoadMeshBones(size_t MeshIndex, const aiMesh* pMesh) {
  for (size_t i = 0; i < pMesh->mNumBones; i++) {
    LoadSingleBone(MeshIndex, pMesh->mBones[i]);
  }
}

void Model::LoadSingleBone(size_t MeshIndex, const aiBone* pBone) {
  auto BoneId = GetBoneId(pBone);

  if (BoneId == m_BoneInfo.size()) {
    BoneInfo bi(convertAssimpMatToGLM(pBone->mOffsetMatrix));
    m_BoneInfo.push_back(bi);
  }

  for (size_t i = 0; i < pBone->mNumWeights; i++) {
    const aiVertexWeight& vw = pBone->mWeights[i];
    size_t GlobalVertexID =
        m_Meshes[MeshIndex].BaseVertex + pBone->mWeights[i].mVertexId;
    m_Bones[GlobalVertexID].AddBoneData(BoneId, vw.mWeight);
  }
}

size_t Model::GetBoneId(const aiBone* pBone) {
  size_t BoneIndex = 0;
  std::string BoneName(pBone->mName.C_Str());

  if (m_BoneNameToIndexMap.find(BoneName) == m_BoneNameToIndexMap.end()) {
    // Allocate an index for a new bone
    BoneIndex                      = (size_t)m_BoneNameToIndexMap.size();
    m_BoneNameToIndexMap[BoneName] = BoneIndex;
  } else {
    BoneIndex = m_BoneNameToIndexMap[BoneName];
  }

  return BoneIndex;
}

void Model::PopulateBuffers() {
  glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(m_Positions[0]) * m_Positions.size(),
               &m_Positions[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(POSITION_LOCATION);
  glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TEXCOORD_VB]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(m_TexCoords[0]) * m_TexCoords.size(),
               &m_TexCoords[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(TEX_COORD_LOCATION);
  glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(m_Normals[0]) * m_Normals.size(),
               &m_Normals[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(NORMAL_LOCATION);
  glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[BONE_VB]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(m_Bones[0]) * m_Bones.size(),
               &m_Bones[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(BONE_ID_LOCATION);
  glVertexAttribIPointer(BONE_ID_LOCATION, MAX_NUM_BONES_PER_VERTEX, GL_INT,
                         sizeof(VertexBoneData), (const GLvoid*)0);
  glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
  glVertexAttribPointer(
      BONE_WEIGHT_LOCATION, MAX_NUM_BONES_PER_VERTEX, GL_FLOAT, GL_FALSE,
      sizeof(VertexBoneData),
      (const GLvoid*)(MAX_NUM_BONES_PER_VERTEX * sizeof(int32_t)));

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_Indices[0]) * m_Indices.size(),
               &m_Indices[0], GL_STATIC_DRAW);
}

// Introduced in youtube tutorial #18
void Model::Render() {
  glBindVertexArray(m_VAO);

  for (unsigned int i = 0; i < m_Meshes.size(); i++) {
    unsigned int MaterialIndex = m_Meshes[i].MaterialIndex;

    // assert(MaterialIndex < m_TextureList.size());
    if (MaterialIndex < m_TextureList.size() &&
        m_TextureList[MaterialIndex] != nullptr) {
      m_TextureList[MaterialIndex]->useTexture();
    } else {
      missingTexture.useTexture();
    }

    glDrawElementsBaseVertex(
        GL_TRIANGLES, m_Meshes[i].NumIndices, GL_UNSIGNED_INT,
        (void*)(sizeof(unsigned int) * m_Meshes[i].BaseIndex),
        m_Meshes[i].BaseVertex);
  }

  // Make sure the VAO is not changed from the outside
  glBindVertexArray(0);
}

const std::string& Model::GetActiveAnimationName() const {
  for (auto& it : m_AnimationNameToIndexMap) {
    if (it.second == m_ActiveAnimation) {
      return it.first;
    }
  }
  // error, animation not found???
  assert(0 && "Animation not found");
}

void Model::SetActiveAnimation(const std::string& animation) {
  if (m_AnimationState == ANIMATION_STATE::IN_ANIMATION) {
    // TODO: here we can handle multiple animations styles
    // e.g.: cancelable animations
    // this solution makes every animation not cancellable
    return;
  }
  // std::println("Setting animation to: {}", animation);
  assert(m_AnimationNameToIndexMap.contains(animation) &&
         "Animation not found");
  m_ActiveAnimation = m_AnimationNameToIndexMap[animation];
  m_AnimationState  = ANIMATION_STATE::IN_ANIMATION;
}

void Model::Animate(GLfloat deltaTime) {
  switch (m_AnimationState) {
  case ANIMATION_STATE::NO_ANIMATION:
    break;
  case ANIMATION_STATE::IN_ANIMATION:
    m_AnimationTime += deltaTime;
    if (m_AnimationTime * m_TicksPerSecond >
        m_Scene->mAnimations[m_ActiveAnimation]->mDuration) {
      m_AnimationState = ANIMATION_STATE::IDLE;
      m_AnimationTime  = 0.f;
    }
    break;
  case ANIMATION_STATE::IDLE:
    m_AnimationTime = 0.f;
    break;
  default:
    break;
  };
}

size_t Model::FindPosition(float AnimationTimeTicks,
                           const aiNodeAnim* pNodeAnim) {
  for (size_t i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
    float t = (float)pNodeAnim->mPositionKeys[i + 1].mTime;
    if (AnimationTimeTicks < t) {
      return i;
    }
  }

  return 0;
}

void Model::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTimeTicks,
                                     const aiNodeAnim* pNodeAnim) {
  // we need at least two values to interpolate...
  if (pNodeAnim->mNumPositionKeys == 1) {
    Out = pNodeAnim->mPositionKeys[0].mValue;
    return;
  }

  size_t PositionIndex     = FindPosition(AnimationTimeTicks, pNodeAnim);
  size_t NextPositionIndex = PositionIndex + 1;
  assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
  float t1        = (float)pNodeAnim->mPositionKeys[PositionIndex].mTime;
  float t2        = (float)pNodeAnim->mPositionKeys[NextPositionIndex].mTime;
  float DeltaTime = t2 - t1;
  float Factor    = (AnimationTimeTicks - t1) / DeltaTime;
  assert(Factor >= 0.0f && Factor <= 1.0f);
  const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
  const aiVector3D& End   = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
  aiVector3D Delta        = End - Start;
  Out                     = Start + Factor * Delta;
}

size_t Model::FindRotation(float AnimationTimeTicks,
                           const aiNodeAnim* pNodeAnim) {
  assert(pNodeAnim->mNumRotationKeys > 0);

  for (size_t i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
    float t = (float)pNodeAnim->mRotationKeys[i + 1].mTime;
    if (AnimationTimeTicks < t) {
      return i;
    }
  }

  return 0;
}

void Model::CalcInterpolatedRotation(aiQuaternion& Out,
                                     float AnimationTimeTicks,
                                     const aiNodeAnim* pNodeAnim) {
  // we need at least two values to interpolate...
  if (pNodeAnim->mNumRotationKeys == 1) {
    Out = pNodeAnim->mRotationKeys[0].mValue;
    return;
  }

  size_t RotationIndex     = FindRotation(AnimationTimeTicks, pNodeAnim);
  size_t NextRotationIndex = RotationIndex + 1;
  assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
  float t1        = (float)pNodeAnim->mRotationKeys[RotationIndex].mTime;
  float t2        = (float)pNodeAnim->mRotationKeys[NextRotationIndex].mTime;
  float DeltaTime = t2 - t1;
  float Factor    = (AnimationTimeTicks - t1) / DeltaTime;
  assert(Factor >= 0.0f && Factor <= 1.0f);
  const aiQuaternion& StartRotationQ =
      pNodeAnim->mRotationKeys[RotationIndex].mValue;
  const aiQuaternion& EndRotationQ =
      pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
  aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
  Out.Normalize();
}

size_t Model::FindScaling(float AnimationTimeTicks,
                          const aiNodeAnim* pNodeAnim) {
  assert(pNodeAnim->mNumScalingKeys > 0);

  for (size_t i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
    float t = (float)pNodeAnim->mScalingKeys[i + 1].mTime;
    if (AnimationTimeTicks < t) {
      return i;
    }
  }

  return 0;
}

void Model::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTimeTicks,
                                    const aiNodeAnim* pNodeAnim) {
  // we need at least two values to interpolate...
  if (pNodeAnim->mNumScalingKeys == 1) {
    Out = pNodeAnim->mScalingKeys[0].mValue;
    return;
  }

  size_t ScalingIndex     = FindScaling(AnimationTimeTicks, pNodeAnim);
  size_t NextScalingIndex = ScalingIndex + 1;
  assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
  float t1        = (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime;
  float t2        = (float)pNodeAnim->mScalingKeys[NextScalingIndex].mTime;
  float DeltaTime = t2 - t1;
  float Factor    = (AnimationTimeTicks - (float)t1) / DeltaTime;
  assert(Factor >= 0.0f && Factor <= 1.0f);
  const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
  const aiVector3D& End   = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
  aiVector3D Delta        = End - Start;
  Out                     = Start + Factor * Delta;
}

void Model::ReadNodeHierarchy(float AnimationTimeTicks, const aiNode* pNode,
                              const glm::mat4& ParentTransform) {
  std::string NodeName(pNode->mName.data);

  const aiAnimation* pAnimation = m_Scene->mAnimations[m_ActiveAnimation];

  glm::mat4 NodeTransformation(convertAssimpMatToGLM(pNode->mTransformation));

  const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);

  if (pNodeAnim) {
    // Interpolate scaling and generate scaling transformation matrix
    aiVector3D Scaling;
    CalcInterpolatedScaling(Scaling, AnimationTimeTicks, pNodeAnim);
    glm::mat4 ScalingM{1.f};
    ScalingM = glm::scale(ScalingM, {Scaling.x, Scaling.y, Scaling.z});
    // std::println("[{}] scaling: {} {} {}",NodeName, Scaling.x, Scaling.y,
    // Scaling.z);
    //  ScalingM.InitScaleTransform(Scaling.x, Scaling.y, Scaling.z);

    // Interpolate rotation and generate rotation transformation matrix
    aiQuaternion RotationQ;
    CalcInterpolatedRotation(RotationQ, AnimationTimeTicks, pNodeAnim);
    glm::quat q;
    q.w                 = RotationQ.w;
    q.x                 = RotationQ.x;
    q.y                 = RotationQ.y;
    q.z                 = RotationQ.z;
    glm::mat4 RotationM = glm::toMat4(q);
    // std::println("[{}] quaternion: {} {} {} {}",NodeName, RotationQ.x,
    // RotationQ.y, RotationQ.z, RotationQ.w); RotationM =
    // convertAssimpMatToGLM(RotationQ.GetMatrix()); RotationM =
    // Matrix4f(RotationQ.GetMatrix());

    // Interpolate translation and generate translation transformation matrix
    aiVector3D Translation;
    CalcInterpolatedPosition(Translation, AnimationTimeTicks, pNodeAnim);
    glm::mat4 TranslationM{1.f};
    TranslationM = glm::translate(
        TranslationM, {Translation.x, Translation.y, Translation.z});
    // std::println("[{}] translation: {} {} {}",NodeName, Translation.x,
    // Translation.y, Translation.z);
    //  TranslationM.InitTranslationTransform(Translation.x, Translation.y,
    //  Translation.z);

    // Combine the above transformations
    NodeTransformation = TranslationM * RotationM * ScalingM;
    // NodeTransformation = glm::mat4{1.f};
  }

  glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;

  if (m_BoneNameToIndexMap.find(NodeName) != m_BoneNameToIndexMap.end()) {
    size_t BoneIndex = m_BoneNameToIndexMap[NodeName];
    m_BoneInfo[BoneIndex].FinalTransformation =
        m_GlobalInverseTransform * GlobalTransformation *
        m_BoneInfo[BoneIndex].OffsetMatrix;
  }

  for (size_t i = 0; i < pNode->mNumChildren; i++) {
    ReadNodeHierarchy(AnimationTimeTicks, pNode->mChildren[i],
                      GlobalTransformation);
  }
}

void Model::GetBoneTransforms(std::vector<glm::mat4>& Transforms) {
  glm::mat4 Identity{1.f};

  m_TicksPerSecond =
      (float)(m_Scene->mAnimations[m_ActiveAnimation]->mTicksPerSecond != 0
                  ? m_Scene->mAnimations[m_ActiveAnimation]->mTicksPerSecond
                  : 25.0f);
  float TimeInTicks        = m_AnimationTime * m_TicksPerSecond;
  float AnimationTimeTicks = fmod(
      TimeInTicks, (float)m_Scene->mAnimations[m_ActiveAnimation]->mDuration);

  ReadNodeHierarchy(AnimationTimeTicks, m_Scene->mRootNode, Identity);
  Transforms.resize(m_BoneInfo.size());

  for (size_t i = 0; i < m_BoneInfo.size(); i++) {
    Transforms[i] = m_BoneInfo[i].FinalTransformation;
  }
}

Model::ANIMATION_STATE Model::GetAnimationState() const {
  return m_AnimationState;
}

float Model::GetAnimationProgress() const {
  return m_AnimationTime * m_TicksPerSecond /
         m_Scene->mAnimations[m_ActiveAnimation]->mDuration;
}

const aiNodeAnim* Model::FindNodeAnim(const aiAnimation* pAnimation,
                                      const std::string& NodeName) {
  for (size_t i = 0; i < pAnimation->mNumChannels; i++) {
    const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

    if (std::string(pNodeAnim->mNodeName.data) == NodeName) {
      return pNodeAnim;
    }
  }
  return NULL;
}

/*
void Model::Render()
{
    glBindVertexArray(m_VAO);

    for (unsigned int i = 0 ; i < m_Meshes.size() ; i++) {
        unsigned int MaterialIndex = m_Meshes[i].MaterialIndex;

        assert(MaterialIndex < m_Materials.size());

        if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE]) {
            m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE]->Bind(COLOR_TEXTURE_UNIT);
        }

        if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR]) {
            m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR]->Bind(SPECULAR_EXPONENT_UNIT);
        }

        glDrawElementsBaseVertex(GL_TRIANGLES,
                                 m_Meshes[i].NumIndices,
                                 GL_UNSIGNED_INT,
                                 (void*)(sizeof(unsigned int) *
m_Meshes[i].BaseIndex), m_Meshes[i].BaseVertex);
    }

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);
}

bool Model::InitMaterials(const std::string& Filename)
{
    std::string Dir = GetDirFromFilename(Filename);

    bool Ret = true;

    printf("Num materials: %d\n", m_Scene->mNumMaterials);

    // Initialize the materials
    for (unsigned int i = 0 ; i < m_Scene->mNumMaterials ; i++) {
        const aiMaterial* pMaterial = m_Scene->mMaterials[i];

        LoadTextures(Dir, pMaterial, i);

        LoadColors(pMaterial, i);
    }

    return Ret;
}

void Model::LoadTextures(const std::string& Dir, const aiMaterial* pMaterial,
int index)
{
    LoadDiffuseTexture(Dir, pMaterial, index);
    LoadSpecularTexture(Dir, pMaterial, index);
}


void Model::LoadDiffuseTexture(const string& Dir, const aiMaterial* pMaterial,
int index)
{
    m_Materials[index].pTextures[TEX_TYPE_BASE] = NULL;

    if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        aiString Path;

        if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL,
NULL, NULL, NULL) == AI_SUCCESS) { string p(Path.data);

            if (p.substr(0, 2) == ".\\") {
                p = p.substr(2, p.size() - 2);
            }

            string FullPath = Dir + "/" + p;

            m_Materials[index].pTextures[TEX_TYPE_BASE] = new
Texture(GL_TEXTURE_2D, FullPath.c_str());

            if (!m_Materials[index].pTextures[TEX_TYPE_BASE]->Load()) {
                printf("Error loading diffuse texture '%s'\n",
FullPath.c_str()); exit(0);
            }
            else {
                printf("Loaded diffuse texture '%s'\n", FullPath.c_str());
            }
        }
    }
}


void Model::LoadSpecularTexture(const string& Dir, const aiMaterial* pMaterial,
int index)
{
    m_Materials[index].pTextures[TEX_TYPE_SPECULAR] = NULL;

    if (pMaterial->GetTextureCount(aiTextureType_SHININESS) > 0) {
        aiString Path;

        if (pMaterial->GetTexture(aiTextureType_SHININESS, 0, &Path, NULL, NULL,
NULL, NULL, NULL) == AI_SUCCESS) { string p(Path.data);

            if (p == "C:\\\\") {
                p = "";
            } else if (p.substr(0, 2) == ".\\") {
                p = p.substr(2, p.size() - 2);
            }

            string FullPath = Dir + "/" + p;

            m_Materials[index].pTextures[TEX_TYPE_SPECULAR] = new
Texture(GL_TEXTURE_2D, FullPath.c_str());

            if (!m_Materials[index].pTextures[TEX_TYPE_SPECULAR]->Load()) {
                printf("Error loading specular texture '%s'\n",
FullPath.c_str()); exit(0);
            }
            else {
                printf("Loaded specular texture '%s'\n", FullPath.c_str());
            }
        }
    }
}

void Model::LoadColors(const aiMaterial* pMaterial, int index)
{
    aiColor4D AmbientColor(0.0f, 0.0f, 0.0f, 0.0f);
    Vector4f AllOnes(1.0f, 1.0f, 1.0f, 1.0);

    int ShadingModel = 0;
    if (pMaterial->Get(AI_MATKEY_SHADING_MODEL, ShadingModel) == AI_SUCCESS) {
        printf("Shading model %d\n", ShadingModel);
    }

    if (pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, AmbientColor) == AI_SUCCESS) {
        printf("Loaded ambient color [%f %f %f]\n", AmbientColor.r,
AmbientColor.g, AmbientColor.b); m_Materials[index].AmbientColor.r =
AmbientColor.r; m_Materials[index].AmbientColor.g = AmbientColor.g;
        m_Materials[index].AmbientColor.b = AmbientColor.b;
    } else {
        m_Materials[index].AmbientColor = AllOnes;
    }

    aiColor3D DiffuseColor(0.0f, 0.0f, 0.0f);

    if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, DiffuseColor) == AI_SUCCESS) {
        printf("Loaded diffuse color [%f %f %f]\n", DiffuseColor.r,
DiffuseColor.g, DiffuseColor.b); m_Materials[index].DiffuseColor.r =
DiffuseColor.r; m_Materials[index].DiffuseColor.g = DiffuseColor.g;
        m_Materials[index].DiffuseColor.b = DiffuseColor.b;
    }

    aiColor3D SpecularColor(0.0f, 0.0f, 0.0f);

    if (pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, SpecularColor) == AI_SUCCESS) {
        printf("Loaded specular color [%f %f %f]\n", SpecularColor.r,
SpecularColor.g, SpecularColor.b); m_Materials[index].SpecularColor.r =
SpecularColor.r; m_Materials[index].SpecularColor.g = SpecularColor.g;
        m_Materials[index].SpecularColor.b = SpecularColor.b;
    }
}
*/
