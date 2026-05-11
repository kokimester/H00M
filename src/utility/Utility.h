#pragma once
#include "FileObserver.h"
#include "Line.h"
#include "Shader.h"
#include <assimp/Importer.hpp>
#include <filesystem>
#include <glm/glm.hpp>
#include <string>
#include <string_view>

void handleGLerrors();

glm::mat4 convertAssimpMatToGLM(const aiMatrix4x4& AssimpMatrix);

glm::mat4 convertAssimpMatToGLM(const aiMatrix3x3& AssimpMatrix);

bool isValidProjectPath(std::filesystem::path& projectPath);

std::unique_ptr<FileObserver> fileObserverFactory();

int loadshader(std::filesystem::path projectPath,
               const std::string& shaderDirStr, std::string_view shaderName,
               Shader& shader);

int validateShaderFiles(const std::filesystem::path& projectPath,
                        const std::filesystem::path& shaderDir,
                        const std::filesystem::path& vertexShaderFile,
                        const std::filesystem::path& fragmentShaderFile,
                        Shader& shader);

template <class Registry>
void renderBoxes(const Registry& componentRegistryBoxes) {
  static unsigned int VAO = 0, VBO = 0, EBO = 0;

  // 12 triangles * 3 indices = 36
  static constexpr unsigned int indices[] = {
      // top    (max.y)
      0,
      1,
      3,
      3,
      2,
      0,
      // bottom (min.y)
      4,
      6,
      7,
      7,
      5,
      4,
      // front  (max.z)
      0,
      4,
      5,
      5,
      1,
      0,
      // back   (min.z)
      2,
      3,
      7,
      7,
      6,
      2,
      // right  (+X, max.x)
      0,
      2,
      6,
      6,
      4,
      0,
      // left   (-X, min.x)
      1,
      5,
      7,
      7,
      3,
      1,
  };

  if (VAO == 0) {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // 8 vertices * (vec3 pos + vec3 color) = 8 * 6 floats, dynamic since box
    // changes
    glBufferData(GL_ARRAY_BUFFER, 8 * 6 * sizeof(float), nullptr,
                 GL_DYNAMIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                 GL_STATIC_DRAW);

    // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)0);
    // color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDepthMask(GL_FALSE);
  glBindVertexArray(VAO);

  for (const auto& [entityID, boxComponent] : componentRegistryBoxes) {
    const auto& max    = boxComponent.box.max;
    const auto& min    = boxComponent.box.min;
    glm::vec3 boxColor = {1.f, 0.f, 0.f};
    boxColor           = boxComponent.color;

    // interleaved: pos(xyz) color(xyz) per vertex, 8 vertices
    float vertexData[] = {
        max.x, max.y, max.z, boxColor.r, boxColor.g, boxColor.b, // 0 top
        min.x, max.y, max.z, boxColor.r, boxColor.g, boxColor.b, // 1
        max.x, max.y, min.z, boxColor.r, boxColor.g, boxColor.b, // 2
        min.x, max.y, min.z, boxColor.r, boxColor.g, boxColor.b, // 3
        max.x, min.y, max.z, boxColor.r, boxColor.g, boxColor.b, // 4 bottom
        min.x, min.y, max.z, boxColor.r, boxColor.g, boxColor.b, // 5
        max.x, min.y, min.z, boxColor.r, boxColor.g, boxColor.b, // 6
        min.x, min.y, min.z, boxColor.r, boxColor.g, boxColor.b, // 7
    };

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexData), vertexData);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
  }

  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
  glEnable(GL_CULL_FACE);
  glBindVertexArray(0);
  /*
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
  boxLines.push_back(l);
  l.updateWithPosition(points[1], points[3], color);
  boxLines.push_back(l);
  l.updateWithPosition(points[3], points[2], color);
  boxLines.push_back(l);
  l.updateWithPosition(points[2], points[0], color);
  boxLines.push_back(l);

  l.updateWithPosition(points[0 + 4], points[1 + 4], color);
  boxLines.push_back(l);
  l.updateWithPosition(points[1 + 4], points[3 + 4], color);
  boxLines.push_back(l);
  l.updateWithPosition(points[3 + 4], points[2 + 4], color);
  boxLines.push_back(l);
  l.updateWithPosition(points[2 + 4], points[0 + 4], color);
  boxLines.push_back(l);

  l.updateWithPosition(points[0], points[0 + 4], color);
  boxLines.push_back(l);
  l.updateWithPosition(points[1], points[1 + 4], color);
  boxLines.push_back(l);
  l.updateWithPosition(points[2], points[2 + 4], color);
  boxLines.push_back(l);
  l.updateWithPosition(points[3], points[3 + 4], color);
  boxLines.push_back(l);

  // cross lines
  l.updateWithPosition(points[0], points[3], color);
  boxLines.push_back(l);
  l.updateWithPosition(points[0], points[5], color);
  boxLines.push_back(l);
  l.updateWithPosition(points[0], points[6], color);
  boxLines.push_back(l);

  l.updateWithPosition(points[7], points[1], color);
  boxLines.push_back(l);
  l.updateWithPosition(points[7], points[2], color);
  boxLines.push_back(l);
  l.updateWithPosition(points[7], points[4], color);
  boxLines.push_back(l);
}
for (auto& line : boxLines) {
  line.render();
}
*/
}
