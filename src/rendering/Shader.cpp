#include "Shader.h"

std::string Shader::loadShaderSource(const char *fileName) {
  std::string temp = "";
  std::string sourceCode = "";

  std::ifstream inFile;
  inFile.open(fileName);

  if (inFile.is_open()) {
    while (std::getline(inFile, temp)) {
      sourceCode += temp + "\n";
    }
  } else {
    std::cerr << "ERROR WHILE OPENING SHADER SOURCODE: " << fileName
              << std::endl;
  }
  inFile.close();

  return sourceCode;
}

GLuint Shader::addShader(const char *fileName,
                         GLenum shaderType) // GLuint program
{
  GLuint theShader = glCreateShader(shaderType);

  const GLchar *theCode;
  std::string preLoad = loadShaderSource(fileName);
  theCode = preLoad.c_str();
  GLint codeLength;
  codeLength = strlen(theCode);

  glShaderSource(theShader, 1, &theCode, &codeLength);
  glCompileShader(theShader);

  GLint result = 0;
  GLchar eLog[1024] = {0x00};

  glGetShaderiv(theShader, GL_COMPILE_STATUS,
                &result); // ellenorizzuk sikerult e
  if (!result) {
    glGetShaderInfoLog(theShader, sizeof(eLog), nullptr, eLog);
    std::cout << "Error compiling " << shaderType << " shader: " << eLog
              << std::endl;
  }
  // glAttachShader(id, theShader);
  return theShader;
}

void Shader::linkProgram(GLuint vertexShader, GLuint fragmentShader) {
  GLchar eLog[1024] = {0x00};
  GLint result;

  id = glCreateProgram();

  if (!id) {
    std::cout << "Error creating shader" << std::endl;
    return;
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
  }

  glValidateProgram(id);                           // validaljuk
  glGetProgramiv(id, GL_VALIDATE_STATUS, &result); // ellenorizzuk sikerult e
  if (!result) {
    glGetProgramInfoLog(id, sizeof(eLog), nullptr, eLog);
    std::cout << "Error validating program: " << eLog << std::endl;
    return;
  }
  // uniformModel = glGetUniformLocation(id, "model");
  // uniformProjection = glGetUniformLocation(id, "projection");
  glUseProgram(0);
}

Shader::Shader(const char *vertexFile, const char *fragmentFile) {
  GLuint vertexShader = 0;
  GLuint fragmentShader = 0;

  id = 0;
  pointLightCount = 0;
  spotLightCount = 0;

  vertexShader = addShader(vertexFile, GL_VERTEX_SHADER);
  fragmentShader = addShader(fragmentFile, GL_FRAGMENT_SHADER);

  linkProgram(vertexShader, fragmentShader);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

void Shader::use() { glUseProgram(id); }

void Shader::unuse() { glUseProgram(0); }

void Shader::clearShader() {
  if (id != 0) {
    glDeleteProgram(id);
    id = 0;
  }
}

void Shader::setSpotLights(Spotlight *arrayToSet, unsigned int spotLightCount) {
  if (spotLightCount > MAX_SPOT_LIGHTS) {
    spotLightCount = MAX_SPOT_LIGHTS;
  }

  set1i(spotLightCount, "spotLightCount");

  for (int i = 0; i < spotLightCount; i++) {
    const int buffsize = 100;
    char locBuff[buffsize] = {0x00};

    snprintf(locBuff, buffsize, "spotLights[%d].base.base.colour", i);
    setVec3f(arrayToSet[i].getColour(), locBuff);

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

    snprintf(locBuff, buffsize, "spotLights[%d].edge", i);
    set1f(arrayToSet[i].getProcEdge(), locBuff);
  }
}

void Shader::setPointLights(PointLight *arrayToSet, unsigned int lightCount) {
  if (lightCount > MAX_POINT_LIGHTS) {
    lightCount = MAX_POINT_LIGHTS;
  }

  set1i(lightCount, "pointLightCount");

  for (int i = 0; i < MAX_POINT_LIGHTS; i++) {
    const int buffsize = 100;
    char locBuff[buffsize] = {0x00};

    snprintf(locBuff, buffsize, "pointLights[%d].base.colour", i);
    setVec3f(arrayToSet[i].getColour(), locBuff);

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

void Shader::setDirectionalLight(DirectionalLight &toSet) {
  setVec3f(toSet.getColour(), "directionalLight.base.colour");
  set1f(toSet.getAmbientIntensity(), "directionalLight.base.ambientIntensity");
  setVec3f(toSet.getDirection(), "directionalLight.direction");
  set1f(toSet.getDiffuseIntensity(), "directionalLight.base.diffuseIntensity");
  // toSet.useLight(uniformDirectionalLight.uniformAmbientIntensity,
  // uniformDirectionalLight.uniformColour,
  //	uniformDirectionalLight.uniformDiffuseIntensity,
  // uniformDirectionalLight.uniformDirection);
}

void Shader::useLight(const DirectionalLight &toUse, const GLchar *colourName,
                      const GLchar *ambientName, const GLchar *directionName,
                      const GLchar *diffuseName) {
  setVec3f(toUse.getColour(), colourName);
  set1f(toUse.getAmbientIntensity(), ambientName);
  setVec3f(toUse.getDirection(), directionName);
  set1f(toUse.getDiffuseIntensity(), diffuseName);
}

void Shader::useMaterial(const Material &toUse, const GLchar *shininessName,
                         const GLchar *specularName) {
  glUniform1f(glGetUniformLocation(id, shininessName), toUse.getShininess());
  glUniform1f(glGetUniformLocation(id, specularName),
              toUse.getSpecularIntensity());
  // set1f(toUse.getShininess(), shininessName);
  // set1f(toUse.getSpecularIntensity(), specularName);
}

void Shader::set1i(GLint value, const GLchar *name) {
  use();

  glUniform1i(glGetUniformLocation(id, name), value);

  unuse();
}

void Shader::set1f(float value, const GLchar *name) {
  use();

  glUniform1f(glGetUniformLocation(id, name), value);

  unuse();
}

void Shader::setVec2f(glm::fvec2 value, const GLchar *name) {
  use();

  glUniform2fv(glGetUniformLocation(id, name), 1, glm::value_ptr(value));

  unuse();
}

void Shader::setVec3f(glm::fvec3 value, const GLchar *name) {
  use();

  glUniform3fv(glGetUniformLocation(id, name), 1, glm::value_ptr(value));

  unuse();
}

void Shader::setVec4f(glm::fvec4 value, const GLchar *name) {
  use();

  glUniform4fv(glGetUniformLocation(id, name), 1, glm::value_ptr(value));

  unuse();
}

void Shader::setMat3fv(glm::mat3 value, const GLchar *name, bool transpose) {
  use();

  glUniformMatrix3fv(glGetUniformLocation(id, name), 1, transpose,
                     glm::value_ptr(value));

  unuse();
}

void Shader::setMat4fv(glm::mat4 value, const GLchar *name, bool transpose) {
  use();

  glUniformMatrix4fv(glGetUniformLocation(id, name), 1, transpose,
                     glm::value_ptr(value));

  unuse();
}
