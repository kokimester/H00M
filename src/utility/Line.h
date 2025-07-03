#pragma once
#include <array>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Line {
public:
  Line();

  void updateWithPosition(const glm::vec3 &, const glm::vec3 &,
                          const glm::vec3 &);
  void updateWithDirection(const glm::vec3 &, const glm::vec3 &,
                           const glm::vec3 &);
  void render();

private:
  static constexpr float LENGTH = 10.f;
  std::array<float, 12> lineData;
  unsigned int VAO, VBO;
};
