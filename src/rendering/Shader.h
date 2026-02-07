#pragma once
#include <fstream>
#include <iostream>
#include <print>
#include <string>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "CommonValues.h"
#include "DirectionalLight.h"
#include "Material.h"
#include "PointLight.h"
#include "Spotlight.h"
#include "Subscriber.h"

class Shader : public Subscriber {
private:
  GLuint m_id = 0;
  std::string loadShaderSource(const char* fileName);
  GLuint addShader(const char* shaderCode, GLenum shaderType);

  std::string m_vertex_path   = "";
  std::string m_fragment_path = "";

  int pointLightCount;
  int spotLightCount;

  static constexpr size_t MAX_BONE_COUNT = 200;

  std::array<GLint, MAX_BONE_COUNT> m_boneLocation = {0};

  struct {
    GLuint uniformColor;
    GLuint uniformAmbientIntensity;
    GLuint uniformDiffuseIntensity;

    GLuint uniformPosition;
    GLuint uniformConstant;
    GLuint uniformLinear;
    GLuint uniformExponent;
  } uniformPointLight[MAX_POINT_LIGHTS];

  struct {
    GLuint uniformColor;
    GLuint uniformAmbientIntensity;
    GLuint uniformDiffuseIntensity;

    GLuint uniformPosition;
    GLuint uniformConstant;
    GLuint uniformLinear;
    GLuint uniformExponent;

    GLuint uniformDirection;
    GLuint uniformEdge;

    GLuint uniformInnerCutoff;
    GLuint uniformOuterCutoff;
  } uniformSpotLight[MAX_SPOT_LIGHTS];

  int linkProgram(GLuint vertexShader, GLuint fragmentShader);

public:
  Shader();
  ~Shader() { glDeleteProgram(m_id); }

  int getID() const { return m_id; }

  int compile_and_link(const char* vertexFile, const char* fragmentFile);
  int reload();

  virtual void notify() override { reload(); }

  void use();
  void unuse();

  void clearShader();

  // Bones
  void setupBones();
  void setBoneTransform(GLint Index, const glm::mat4& Transform);
  // Bones

  void setSpotLights(Spotlight* arrayToSet, unsigned int spotLightCount);
  void setPointLights(PointLight* arrayToSet, unsigned int lightCount);
  void setDirectionalLight(DirectionalLight& toSet);

  void useLight(const DirectionalLight& toUse, const GLchar* colorName,
                const GLchar* ambientName, const GLchar* directionName,
                const GLchar* diffuseName);
  void useMaterial(const Material& toUse, const GLchar* shininessName,
                   const GLchar* specularName);

  void set1i(GLint value, const GLchar* name);
  void set1f(GLfloat value, const GLchar* name);
  void set3f(glm::fvec3 value, const GLchar* name);
  void setVec2f(glm::fvec2 value, const GLchar* name);
  void setVec3f(glm::fvec3 value, const GLchar* name);
  void setVec4f(glm::fvec4 value, const GLchar* name);
  void setMat3fv(glm::mat3 value, const GLchar* name,
                 bool transpose = GL_FALSE);
  void setMat4fv(glm::mat4 value, const GLchar* name,
                 bool transpose = GL_FALSE);
};
