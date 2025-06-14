#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <exception>
#include <stdexcept>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Texture.h"

//===================
//		EXCEPTIONS
//		std::invalid_argument <- FAILED TO LOAD MODEL
//
//===================
class Model
{
private:
	std::vector<Mesh*> meshList;
	std::vector<Texture*> textureList;
	std::vector<unsigned int> meshToTex;
public:
	Model();
	~Model();
	
	void loadModel(const std::string& fileLocation);
	void renderModel();
	void clearModel();

private:

	void loadNode(aiNode* node, const aiScene* scene);
	void loadMesh(aiMesh* mesh, const aiScene* scene);
	void loadMaterials(const aiScene* scene);


};

