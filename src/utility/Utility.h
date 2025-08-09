#pragma once
#include <assimp/Importer.hpp>
#include <glm/glm.hpp>
#include "Shader.h"
#include <filesystem>
#include <string>

void handleGLerrors();

glm::mat4
convertAssimpMatToGLM(const aiMatrix4x4& AssimpMatrix);

glm::mat4
convertAssimpMatToGLM(const aiMatrix3x3& AssimpMatrix);

bool
isValidProjectPath(std::filesystem::path& projectPath);

int 
loadshader(
std::filesystem::path projectPath,
const std::string& shaderDirStr, const std::string& shader_name,
Shader& shader);

int
validateShaderFiles(
const std::filesystem::path& projectPath,
const std::filesystem::path& shaderDir,
const std::filesystem::path& vertexShaderFile,
const std::filesystem::path& fragmentShaderFile, Shader& shader);