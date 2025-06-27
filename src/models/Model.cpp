#include "Model.h"

Model::Model() {}

Model::~Model() { clearModel(); }

void Model::loadModel(const std::string &fileLocation, bool isSmoothShaded) {
  Assimp::Importer importer;
  // aiProcess_GenSmoothNormals
  // aiProcess_JoinIdenticalVertices
  unsigned int flags = aiProcess_Triangulate | aiProcess_FlipUVs;
  if (isSmoothShaded) {
    flags |= aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices;
    flags |= aiProcess_DropNormals | aiProcess_ForceGenNormals;
  } else {
    flags |= aiProcess_GenNormals;
  }
  const aiScene *scene = importer.ReadFile(fileLocation, flags);
  if (!scene) {
    std::stringstream errS;
    errS << "Model " << fileLocation
         << " failed to load: " << importer.GetErrorString() << std::endl;
    throw std::invalid_argument(errS.str());
  }
  loadNode(scene->mRootNode, scene);
  std::cout << "Loading model " << fileLocation << std::endl;
  loadMaterials(scene);
}

void Model::renderModel(bool useTexture) {
  for (size_t i = 0; i < meshList.size(); i++) {
    if (useTexture) {
      unsigned int materialIndex = meshToTex[i];

      if ((materialIndex < textureList.size()) &&
          (textureList[materialIndex])) {
        textureList[materialIndex]->useTexture();
      }
    }

    meshList[i]->renderMesh();
  }
}

void Model::clearModel() {
  for (size_t i = 0; i < meshList.size(); i++) {
    if (meshList[i]) {
      delete meshList[i];
      meshList[i] = nullptr;
    }
  }
  for (size_t i = 0; i < textureList.size(); i++) {
    if (textureList[i]) {
      delete textureList[i];
      textureList[i] = nullptr;
    }
  }
}

void Model::loadNode(aiNode *node, const aiScene *scene) {
  for (size_t i = 0; i < node->mNumMeshes; i++) {
    loadMesh(scene->mMeshes[node->mMeshes[i]], scene);
  }

  for (size_t i = 0; i < node->mNumChildren; i++) {
    loadNode(node->mChildren[i], scene);
  }
}

void Model::loadMesh(aiMesh *mesh, const aiScene *scene) {
  std::vector<GLfloat> vertices;
  std::vector<unsigned int> indices;
  GLfloat avgx = 0.f, avgy = 0.f, avgz = 0.f;

  for (size_t i = 0; i < mesh->mNumVertices; i++) {
    vertices.insert(vertices.end(), {mesh->mVertices[i].x, mesh->mVertices[i].y,
                                     mesh->mVertices[i].z});
    avgx += mesh->mVertices[i].x;
    avgy += mesh->mVertices[i].y;
    avgz += mesh->mVertices[i].z;
    if (mesh->mTextureCoords[0]) {
      vertices.insert(vertices.end(), {mesh->mTextureCoords[0][i].x,
                                       mesh->mTextureCoords[0][i].y});
    } else {
      /* vertices.insert(vertices.end(), {0.1f * i, 0.2f * i}); */
      vertices.insert(vertices.end(), {0.f, 0.f});
    }
    vertices.insert(vertices.end(), {-mesh->mNormals[i].x, -mesh->mNormals[i].y,
                                     -mesh->mNormals[i].z});
  }
  avgx /= mesh->mNumVertices;
  avgy /= mesh->mNumVertices;
  avgz /= mesh->mNumVertices;

  // optional: center vertices around average of all vertices
#if 1
  constexpr size_t sizeOfVertexData = 8;
  for (size_t i = 0; i < vertices.size(); i += sizeOfVertexData) {
    vertices[i] -= avgx;
    vertices[i + 1] -= avgy;
    vertices[i + 2] -= avgz;
  }
#endif
  // optional: scale down vertex coordinates to 1.0f

  for (size_t i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (size_t j = 0; j < face.mNumIndices; j++) {
      indices.push_back(face.mIndices[j]);
    }
  }

  Mesh *newMesh = new Mesh();
  newMesh->createMesh(&vertices[0], &indices[0], vertices.size(),
                      indices.size());
  meshList.push_back(newMesh);
  meshToTex.push_back(mesh->mMaterialIndex);
}

void Model::loadMaterials(const aiScene *scene) {
  textureList.resize(scene->mNumMaterials);
  /* std::cout << "Material count: " << scene->mNumMaterials << std::endl; */

  for (size_t i = 0; i < scene->mNumMaterials; i++) {
    aiMaterial *material = scene->mMaterials[i];

    /* std::cout << material->GetName().data << std::endl; */
    textureList[i] = nullptr;

    if (material->GetTextureCount(aiTextureType_DIFFUSE)) {
      aiString path;
      if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) {
        int idx = std::string(path.data).rfind('\\');
        std::string fileName = std::string(path.data).substr(idx + 1);

        std::string texPath = std::string("textures/") + fileName;

        textureList[i] = new Texture(texPath.c_str());

        if (!textureList[i]->loadTexture()) {
          std::cerr << "Failed to load texture at: " << texPath << std::endl;
          delete textureList[i];
          textureList[i] = nullptr;
        }
      }
    }
    if (!textureList[i]) {
      textureList[i] = new Texture("../textures/missing.jpg");
      textureList[i]->loadTexture();
    }
  }
}
