#include "Shader.h"
#include "Utility.h"

std::string Shader::loadShaderSource(const char* fileName) {
  std::string temp       = "";
  std::string sourceCode = "";

  std::ifstream inFile;
  inFile.open(fileName);

  if (inFile.is_open()) {
    while (std::getline(inFile, temp)) {
      sourceCode += temp + "\n";
    }
  } else {
    std::cerr << "ERROR WHILE OPENING SHADER SOURCE CODE: " << fileName
              << std::endl;
  }
  inFile.close();

  // TODO: add to log
  /* std::cout << "Shader interpreted as: \n" << sourceCode; */

  return sourceCode;
}

GLuint Shader::addShader(const char* fileName, GLenum shaderType) {
  GLuint theShader = glCreateShader(shaderType);

  const GLchar* theCode;
  std::string preLoad = loadShaderSource(fileName);
  theCode             = preLoad.c_str();
  GLint codeLength;
  codeLength = strlen(theCode);

  glShaderSource(theShader, 1, &theCode, &codeLength);
  glCompileShader(theShader);

  GLint result      = 0;
  GLchar eLog[1024] = {0x00};

  glGetShaderiv(theShader, GL_COMPILE_STATUS, &result);
  // ellenorizzuk sikerult e
  if (!result) {
    glGetShaderInfoLog(theShader, sizeof(eLog), nullptr, eLog);
    std::cout << "Error compiling " << shaderType << " shader: " << eLog
              << std::endl;
  }
  // glAttachShader(m_id, theShader);
  // std::println("Shader compilation: {}", fileName);
  handleGLerrors();
  return theShader;
}

int Shader::linkProgram(GLuint vertexShader, GLuint fragmentShader) {
  GLchar eLog[1024] = {0x00};
  GLint result;

  GLuint id = glCreateProgram();

  if (!id) {
    std::cout << "Error creating shader" << std::endl;
    return 0;
  }

  //-----

  // addShader(vertexLoc, GL_VERTEX_SHADER);
  // addShader(fragmentLoc, GL_FRAGMENT_SHADER);

  //-----

  glAttachShader(id, vertexShader);

  // geometry shader

  glAttachShader(id, fragmentShader);

  glLinkProgram(id);
  glGetProgramiv(id, GL_LINK_STATUS, &result); // ellenorizzuk sikerult e
  if (!result) {
    glGetProgramInfoLog(id, sizeof(eLog), nullptr, eLog);
    std::cout << "Error linking program :" << eLog << std::endl;
    return 0;
  }

  glValidateProgram(id);                           // validaljuk
  glGetProgramiv(id, GL_VALIDATE_STATUS, &result); // ellenorizzuk sikerult e
  if (!result) {
    glGetProgramInfoLog(id, sizeof(eLog), nullptr, eLog);
    std::cout << "Error validating program: " << eLog << std::endl;
    return 0;
  }
  // uniformModel = glGetUniformLocation(id, "model");
  // uniformProjection = glGetUniformLocation(id, "projection");
  // std::println("Shader linking");
  handleGLerrors();
  glUseProgram(0);
  return id;
}

Shader::Shader() {}

int Shader::reload(void) {
  assert((m_vertex_path != "") && (m_fragment_path != ""));
  GLuint vertexShader   = 0;
  GLuint fragmentShader = 0;

  GLuint id       = 0;
  pointLightCount = 0;
  spotLightCount  = 0;

  vertexShader   = addShader(m_vertex_path.c_str(), GL_VERTEX_SHADER);
  fragmentShader = addShader(m_fragment_path.c_str(), GL_FRAGMENT_SHADER);

  id = linkProgram(vertexShader, fragmentShader);

  if (!id) {
    std::println("Error while reloading shader program");
    return -1;
  }

  clearShader();

  m_id = id;

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  return 0;
}

int Shader::compile_and_link(const char* vertexFile, const char* fragmentFile) {
  m_vertex_path   = vertexFile;
  m_fragment_path = fragmentFile;
  auto result     = reload();
  return result;
}

void Shader::use() { glUseProgram(m_id); }

void Shader::unuse() { glUseProgram(0); }

void Shader::clearShader() {
  if (m_id != 0) {
    glDeleteProgram(m_id);
    m_id = 0;
  }
}

void Shader::setSpotLights(Spotlight* arrayToSet, unsigned int spotLightCount) {
  if (spotLightCount > MAX_SPOT_LIGHTS) {
    spotLightCount = MAX_SPOT_LIGHTS;
  }

  set1i(spotLightCount, "spotLightCount");

  for (unsigned int i = 0; i < spotLightCount; i++) {
    const int buffsize     = 100;
    char locBuff[buffsize] = {0x00};

    snprintf(locBuff, buffsize, "spotLights[%d].base.base.color", i);
    setVec3f(arrayToSet[i].getColor(), locBuff);

    snprintf(locBuff, buffsize, "spotLights[%d].base.base.ambientIntensity", i);
    set1f(arrayToSet[i].getAmbientIntensity(), locBuff);

    snprintf(locBuff, buffsize, "spotLights[%d].base.base.diffuseIntensity", i);
    set1f(arrayToSet[i].getDiffuseIntensity(), locBuff);

    snprintf(locBuff, buffsize, "spotLights[%d].base.position", i);
    setVec3f(arrayToSet[i].getPos(), locBuff);

    snprintf(locBuff, buffsize, "spotLights[%d].base.constant", i);
    set1f(arrayToSet[i].getConst(), locBuff);

    snprintf(locBuff, buffsize, "spotLights[%d].base.linear", i);
    set1f(arrayToSet[i].getLinear(), locBuff);

    snprintf(locBuff, buffsize, "spotLights[%d].base.exponent", i);
    set1f(arrayToSet[i].getExp(), locBuff);

    snprintf(locBuff, buffsize, "spotLights[%d].direction", i);
    setVec3f(arrayToSet[i].getDirection(), locBuff);

    // TODO: maybe unused, can remove?
    snprintf(locBuff, buffsize, "spotLights[%d].edge", i);
    set1f(arrayToSet[i].getProcEdge(), locBuff);

    snprintf(locBuff, buffsize, "spotLights[%d].innerCutoff", i);
    set1f(arrayToSet[i].getInnerCutoff(), locBuff);

    snprintf(locBuff, buffsize, "spotLights[%d].outerCutoff", i);
    set1f(arrayToSet[i].getOuterCutoff(), locBuff);
  }
}

void Shader::setPointLights(PointLight* arrayToSet, unsigned int lightCount) {
  if (lightCount > MAX_POINT_LIGHTS) {
    lightCount = MAX_POINT_LIGHTS;
  }

  set1i(lightCount, "pointLightCount");

  for (int i = 0; i < MAX_POINT_LIGHTS; i++) {
    const int buffsize     = 100;
    char locBuff[buffsize] = {0x00};

    snprintf(locBuff, buffsize, "pointLights[%d].base.color", i);
    setVec3f(arrayToSet[i].getColor(), locBuff);

    snprintf(locBuff, buffsize, "pointLights[%d].base.ambientIntensity", i);
    set1f(arrayToSet[i].getAmbientIntensity(), locBuff);

    snprintf(locBuff, buffsize, "pointLights[%d].base.diffuseIntensity", i);
    set1f(arrayToSet[i].getDiffuseIntensity(), locBuff);

    snprintf(locBuff, buffsize, "pointLights[%d].position", i);
    setVec3f(arrayToSet[i].getPos(), locBuff);

    snprintf(locBuff, buffsize, "pointLights[%d].constant", i);
    set1f(arrayToSet[i].getConst(), locBuff);

    snprintf(locBuff, buffsize, "pointLights[%d].linear", i);
    set1f(arrayToSet[i].getLinear(), locBuff);

    snprintf(locBuff, buffsize, "pointLights[%d].exponent", i);
    set1f(arrayToSet[i].getExp(), locBuff);
  }
}

void Shader::setDirectionalLight(DirectionalLight& toSet) {
  setVec3f(toSet.getColor(), "directionalLight.base.color");
  set1f(toSet.getAmbientIntensity(), "directionalLight.base.ambientIntensity");
  setVec3f(toSet.getDirection(), "directionalLight.direction");
  set1f(toSet.getDiffuseIntensity(), "directionalLight.base.diffuseIntensity");
  // toSet.useLight(uniformDirectionalLight.uniformAmbientIntensity,
  // uniformDirectionalLight.uniformColor,
  //	uniformDirectionalLight.uniformDiffuseIntensity,
  // uniformDirectionalLight.uniformDirection);
}

void Shader::useLight(const DirectionalLight& toUse, const GLchar* colorName,
                      const GLchar* ambientName, const GLchar* directionName,
                      const GLchar* diffuseName) {
  setVec3f(toUse.getColor(), colorName);
  set1f(toUse.getAmbientIntensity(), ambientName);
  setVec3f(toUse.getDirection(), directionName);
  set1f(toUse.getDiffuseIntensity(), diffuseName);
}

void Shader::useMaterial(const Material& toUse, const GLchar* shininessName,
                         const GLchar* specularName) {
  glUniform1f(glGetUniformLocation(m_id, shininessName), toUse.getShininess());
  glUniform1f(glGetUniformLocation(m_id, specularName),
              toUse.getSpecularIntensity());
  // set1f(toUse.getShininess(), shininessName);
  // set1f(toUse.getSpecularIntensity(), specularName);
}

void Shader::setupBones() {
  use();
  for (unsigned int i = 0; i < m_boneLocation.size(); i++) {
    char Name[256] = {0x00};
    snprintf(Name, sizeof(Name), "gBones[%d]", i);
    m_boneLocation[i] = glGetUniformLocation(m_id, Name);
    if (m_boneLocation[i] == -1) {
      printf("ERROR: Uniform '%s' not found or optimized out!\n", Name);
    }
  }
}

void Shader::setBoneTransform(GLint Index, const glm::mat4& Transform) {
  assert((Index < static_cast<int>(MAX_BONE_COUNT)) &&
         "Index < MAX_BONE_COUNT failed");

  use();
  glUniformMatrix4fv(m_boneLocation[Index], 1, GL_FALSE,
                     glm::value_ptr(Transform));
  unuse();
}

void Shader::set1i(GLint value, const GLchar* name) {
  use();

  glUniform1i(glGetUniformLocation(m_id, name), value);

  unuse();
}

void Shader::set1f(GLfloat value, const GLchar* name) {
  use();

  glUniform1f(glGetUniformLocation(m_id, name), value);

  unuse();
}

void Shader::set3f(glm::fvec3 value, const GLchar* name) {
  use();

  glUniform3f(glGetUniformLocation(m_id, name), value.x, value.y, value.z);

  unuse();
}

void Shader::setVec2f(glm::fvec2 value, const GLchar* name) {
  use();

  glUniform2fv(glGetUniformLocation(m_id, name), 1, glm::value_ptr(value));

  unuse();
}

void Shader::setVec3f(glm::fvec3 value, const GLchar* name) {
  use();

  glUniform3fv(glGetUniformLocation(m_id, name), 1, glm::value_ptr(value));

  unuse();
}

void Shader::setVec4f(glm::fvec4 value, const GLchar* name) {
  use();

  glUniform4fv(glGetUniformLocation(m_id, name), 1, glm::value_ptr(value));

  unuse();
}

void Shader::setMat3fv(glm::mat3 value, const GLchar* name, bool transpose) {
  use();

  glUniformMatrix3fv(glGetUniformLocation(m_id, name), 1, transpose,
                     glm::value_ptr(value));

  unuse();
}

void Shader::setMat4fv(glm::mat4 value, const GLchar* name, bool transpose) {
  use();

  glUniformMatrix4fv(glGetUniformLocation(m_id, name), 1, transpose,
                     glm::value_ptr(value));

  unuse();
}
