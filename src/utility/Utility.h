#pragma once
#include <assimp/Importer.hpp>
#include <glm/glm.hpp>

void handleGLerrors();

glm::mat4
convertAssimpMatToGLM(const aiMatrix4x4& AssimpMatrix);

glm::mat4
convertAssimpMatToGLM(const aiMatrix3x3& AssimpMatrix);