#pragma once
#include <array>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Line {
public:
  Line();

  void updateWithPosition(const glm::vec3&, const glm::vec3&, const glm::vec3&);
  void updateWithDirection(const glm::vec3&, const glm::vec3&,
                           const glm::vec3&);
  void render();

  const glm::vec3& getStart() const;
  const glm::vec3& getEnd() const;

  float getLength() const;

private:
  glm::vec3 start = glm::vec3{0.f}, end = glm::vec3{0.f};
  static constexpr float LENGTH = 10.f;
  std::array<float, 12> lineData;
  unsigned int VAO, VBO;
};
