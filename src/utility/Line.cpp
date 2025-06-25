#include "Line.h"

Line::Line() : VAO{0}, VBO{0} {
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, lineData.size() * sizeof(float),
               lineData.data(), GL_DYNAMIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void *)(3 * sizeof(float)));
}

void Line::updateWithPosition(const glm::vec3 &p1, const glm::vec3 &p2,
                              const glm::vec3 &color) {
  uint idx = 0;
  // point 1
  lineData[idx++] = (p1.x);
  lineData[idx++] = (p1.y);
  lineData[idx++] = (p1.z);

  // color
  lineData[idx++] = (color.r);
  lineData[idx++] = (color.g);
  lineData[idx++] = (color.b);

  // point 2
  lineData[idx++] = (p2.x);
  lineData[idx++] = (p2.y);
  lineData[idx++] = (p2.z);

  // color
  lineData[idx++] = (color.r);
  lineData[idx++] = (color.g);
  lineData[idx++] = (color.b);
}
void Line::updateWithDirection(const glm::vec3 &from,
                               const glm::vec3 &direction,
                               const glm::vec3 &color) {
  glm::vec3 end = from + Line::LENGTH * direction;
  updateWithPosition(from, end, color);
}
void Line::render() {
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, lineData.size() * sizeof(float),
               lineData.data(), GL_DYNAMIC_DRAW);
  int count = lineData.size() / 6;
  glBindVertexArray(VAO);
  glDrawArrays(GL_LINES, 0, count);
}
