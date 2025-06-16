#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <GLFW/glfw3.h>
#include <filesystem>
#include <glm/glm.hpp>
#include <map>
#include <string_view>

// include shader for text shader
#include "Shader.h"

namespace fs = std::filesystem;

class TextRenderer {
public:
  TextRenderer(GLuint screenWidth, GLuint screenHeight);
  int init(const fs::path &vertexShaderPath, const fs::path &fragmenShaderPath,
           const fs::path &fontPath);
  void renderText(std::string_view text, float x, float y, float scale,
                  glm::vec3 color);

  void updateScreenParameters(GLuint screenWidth, GLuint screenHeight);

private:
  Shader shader;
  GLuint width, height;
  GLuint VAO, VBO;
  struct Character {
    unsigned int TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    unsigned int Advance;
  };
  std::map<char, Character> Characters;
};
