#pragma once
#include "FileObserver.h"
#include "Line.h"
#include "Shader.h"
#include <assimp/Importer.hpp>
#include <filesystem>
#include <glm/glm.hpp>
#include <string>

void handleGLerrors();

glm::mat4 convertAssimpMatToGLM(const aiMatrix4x4& AssimpMatrix);

glm::mat4 convertAssimpMatToGLM(const aiMatrix3x3& AssimpMatrix);

bool isValidProjectPath(std::filesystem::path& projectPath);

std::unique_ptr<FileObserver> fileObserverFactory();

int loadshader(std::filesystem::path projectPath,
               const std::string& shaderDirStr, const std::string& shader_name,
               Shader& shader);

int validateShaderFiles(const std::filesystem::path& projectPath,
                        const std::filesystem::path& shaderDir,
                        const std::filesystem::path& vertexShaderFile,
                        const std::filesystem::path& fragmentShaderFile,
                        Shader& shader);

template <class Registry>
void renderCollisionBoxes(const Registry& componentRegistry) {
  // render collision boxes
  std::vector<Line> collisionBoxLines;
  for (const auto& [entityID, collisionBox] :
       componentRegistry.collision_boxes) {
    // no collision color
    glm::vec3 color = {0.f, 1.f, 0.f};
    if (collisionBox.isColliding) {
      color = {1.f, 0.f, 0.f};
    }
    auto max = collisionBox.box.max;
    auto min = collisionBox.box.min;
    std::array<glm::vec3, 8> points;
    points[0] = {max.x, max.y, max.z};
    points[1] = {min.x, max.y, max.z};
    points[2] = {max.x, max.y, min.z};
    points[3] = {min.x, max.y, min.z};

    points[4] = {max.x, min.y, max.z};
    points[5] = {min.x, min.y, max.z};
    points[6] = {max.x, min.y, min.z};
    points[7] = {min.x, min.y, min.z};
    Line l;
    l.updateWithPosition(points[0], points[1], color);
    collisionBoxLines.push_back(l);
    l.updateWithPosition(points[1], points[3], color);
    collisionBoxLines.push_back(l);
    l.updateWithPosition(points[3], points[2], color);
    collisionBoxLines.push_back(l);
    l.updateWithPosition(points[2], points[0], color);
    collisionBoxLines.push_back(l);

    l.updateWithPosition(points[0 + 4], points[1 + 4], color);
    collisionBoxLines.push_back(l);
    l.updateWithPosition(points[1 + 4], points[3 + 4], color);
    collisionBoxLines.push_back(l);
    l.updateWithPosition(points[3 + 4], points[2 + 4], color);
    collisionBoxLines.push_back(l);
    l.updateWithPosition(points[2 + 4], points[0 + 4], color);
    collisionBoxLines.push_back(l);

    l.updateWithPosition(points[0], points[0 + 4], color);
    collisionBoxLines.push_back(l);
    l.updateWithPosition(points[1], points[1 + 4], color);
    collisionBoxLines.push_back(l);
    l.updateWithPosition(points[2], points[2 + 4], color);
    collisionBoxLines.push_back(l);
    l.updateWithPosition(points[3], points[3 + 4], color);
    collisionBoxLines.push_back(l);

    // cross lines
    l.updateWithPosition(points[0], points[3], color);
    collisionBoxLines.push_back(l);
    l.updateWithPosition(points[0], points[5], color);
    collisionBoxLines.push_back(l);
    l.updateWithPosition(points[0], points[6], color);
    collisionBoxLines.push_back(l);

    l.updateWithPosition(points[7], points[1], color);
    collisionBoxLines.push_back(l);
    l.updateWithPosition(points[7], points[2], color);
    collisionBoxLines.push_back(l);
    l.updateWithPosition(points[7], points[4], color);
    collisionBoxLines.push_back(l);
  }
  for (auto& line : collisionBoxLines) {
    line.render();
  }
}
