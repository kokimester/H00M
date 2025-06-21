#pragma once

#include <exception>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

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
class Model {
private:
  std::vector<Mesh *> meshList;
  std::vector<Texture *> textureList;
  std::vector<unsigned int> meshToTex;

public:
  Model();
  ~Model();

  void loadModel(const std::string &fileLocation, bool isSmoothShaded = true);
  void renderModel(bool useTexture = true);
  void clearModel();

private:
  void loadNode(aiNode *node, const aiScene *scene);
  void loadMesh(aiMesh *mesh, const aiScene *scene);
  void loadMaterials(const aiScene *scene);
};
