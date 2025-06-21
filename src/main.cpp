#define STB_IMAGE_IMPLEMENTATION
/* #include "stb_image.h" */

#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>

#include <cmath>
#include <exception>
#include <math.h>
#include <stdexcept>
#include <stdlib.h>
#include <vector>

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Camera.h"
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "Material.h"
#include "Mesh.h"
#include "Model.h"
#include "PointLight.h"
#include "Shader.h"
#include "Spotlight.h"
#include "Text.h"
#include "Texture.h"
#include "Window.h"

#include <assimp/Importer.hpp>

// TODO: Globals, move somewhere else, as static perhaps
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
GLfloat lastTimeTextWasRendered = 0.0f;
std::string timeStr, FPSStr;
std::string cameraLocStr, cameraFacingStr, entityCountStr;

std::vector<Mesh> meshList;

bool direction = true;
float triOffset = 0.0f;
float triMaxOffset = 0.8f;
float triIncrement = 0.0005f;
float triRotIncrement = 0.02f;
float triRotation = 0.0f;

void calcAverageNormals(unsigned int *indices, unsigned int indiceCount,
                        GLfloat *vertices, unsigned int verticeCount,
                        unsigned int vLength, unsigned int normalOffset) {
  for (unsigned int i = 0; i < indiceCount; i += 3) {
    unsigned int in0 = indices[i] * vLength;
    unsigned int in1 = indices[i + 1] * vLength;
    unsigned int in2 = indices[i + 2] * vLength;
    unsigned int normalIndex0 = indices[i] * vLength + normalOffset;
    unsigned int normalIndex1 = indices[i + 1] * vLength + normalOffset;
    unsigned int normalIndex2 = indices[i + 2] * vLength + normalOffset;
    glm::vec3 v1(vertices[in1] - vertices[in0],
                 vertices[in1 + 1] - vertices[in0 + 1],
                 vertices[in1 + 2] - vertices[in0 + 2]);
    glm::vec3 v2(vertices[in2] - vertices[in0],
                 vertices[in2 + 1] - vertices[in0 + 1],
                 vertices[in2 + 2] - vertices[in0 + 2]);

    glm::vec3 normal = glm::cross(v1, v2);
    normal = glm::normalize(normal);

    vertices[normalIndex0] += normal.x;
    vertices[normalIndex0 + 1] += normal.y;
    vertices[normalIndex0 + 2] += normal.z;

    vertices[normalIndex1] += normal.x;
    vertices[normalIndex1 + 1] += normal.y;
    vertices[normalIndex1 + 2] += normal.z;

    vertices[normalIndex2] += normal.x;
    vertices[normalIndex2 + 1] += normal.y;
    vertices[normalIndex2 + 2] += normal.z;
  }
  for (unsigned int i = 0; i < verticeCount / vLength; i++) {
    unsigned int nOffset = i * vLength + normalOffset;
    glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1],
                  vertices[nOffset + 2]);
    vec = glm::normalize(vec);
    vertices[nOffset] = vec.x;
    vertices[nOffset + 1] = vec.y;
    vertices[nOffset + 2] = vec.z;
  }
}

void createTriangle() {
  unsigned int indices[] = {0, 9, 3, 4, 10, 6, 7, 11, 1, 2, 5, 8};

  GLfloat vertices[] = {
      // x     y     z		u     v   nx    ny    nz
      -1.0f, -1.0f, -0.6f, 0.0f,  0.0f, 0.0f,  0.0f,  0.0f,  -1.0f, -1.0f,
      -0.6f, 0.0f,  0.0f,  0.0f,  0.0f, 0.0f,  -1.0f, -1.0f, -0.6f, 0.0f,
      0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f, 1.0f,  0.5f,  0.0f,  0.0f,
      0.0f,  0.0f,  0.0f,  -1.0f, 1.0f, 0.5f,  0.0f,  0.0f,  0.0f,  0.0f,
      0.0f,  -1.0f, 1.0f,  0.5f,  0.0f, 0.0f,  0.0f,  0.0f,  1.0f,  -1.0f,
      -0.6f, 1.0f,  0.0f,  0.0f,  0.0f, 0.0f,  1.0f,  -1.0f, -0.6f, 1.0f,
      0.0f,  0.0f,  0.0f,  0.0f,  1.0f, -1.0f, -0.6f, 1.0f,  0.0f,  0.0f,
      0.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.5f,  1.0f,  0.0f,  0.0f,  0.0f,
      0.0f,  1.0f,  0.0f,  0.5f,  1.0f, 0.0f,  0.0f,  0.0f,  0.0f,  1.0f,
      0.0f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f};

  // uj objektum

  unsigned int floorIndices[]{0, 1, 2, 1, 2, 3};

  GLfloat floorVertices[]{-10.0f, 0.0f, -10.f, 0.0f,  0.0f,  0.0f, -1.0f, 0.0f,
                          10.0f,  0.0f, -10.f, 10.0f, 0.0f,  0.0f, -1.0f, 0.0f,
                          -10.0f, 0.0f, 10.f,  0.0f,  10.0f, 0.0f, -1.0f, 0.0f,
                          10.0f,  0.0f, 10.f,  10.0f, 10.0f, 0.0f, -1.0f, 0.0f};

  calcAverageNormals(indices, sizeof(indices) / sizeof(unsigned int), vertices,
                     sizeof(vertices) / sizeof(GLfloat), 8, 5);

  meshList.push_back(Mesh());
  meshList.push_back(Mesh());
  meshList.push_back(Mesh());

  meshList[0].createMesh(vertices, indices, sizeof(vertices) / sizeof(GLfloat),
                         sizeof(indices) / sizeof(unsigned int));
  meshList[1].createMesh(vertices, indices, sizeof(vertices) / sizeof(GLfloat),
                         sizeof(indices) / sizeof(unsigned int));
  meshList[2].createMesh(floorVertices, floorIndices,
                         sizeof(floorVertices) / sizeof(GLfloat),
                         sizeof(floorIndices) / sizeof(unsigned int));
#if 0
  unsigned int lenVertices = sizeof(vertices) / sizeof(GLfloat);
  for (unsigned i = 0; i < lenVertices; i = i + 8) {
    printf("v %1.1f %1.1f %1.1f\n", vertices[i], vertices[i + 1],
           vertices[i + 2]);
  }
#endif
#if 0
  for (unsigned i = 6; i < lenVertices; i = i + 8) {
    printf("vn %1.1f %1.1f %1.1f\n", vertices[i], vertices[i + 1],
           vertices[i + 2]);
  }
  for (unsigned i = 3; i < lenVertices; i = i + 8) {
    printf("vt %1.1f %1.1f\n", vertices[i], vertices[i + 1]);
  }
  unsigned int lenIndices = sizeof(indices) / sizeof(unsigned int);
  for (unsigned i = 0; i < lenIndices; i = i + 3) {
    printf("f %d/%d/%d %d/%d/%d %d/%d/%d\n", indices[i] + 1, indices[i] + 1,
           indices[i] + 1, indices[i + 1] + 1, indices[i + 1] + 1,
           indices[i + 1] + 1, indices[i + 2] + 1, indices[i + 2] + 1,
           indices[i + 2] + 1);
  }
#else
#if 0
  unsigned int lenIndices = sizeof(indices) / sizeof(unsigned int);
  for (unsigned i = 0; i < lenIndices; i = i + 3) {
    printf("f %d/%d/ %d/%d/ %d/%d/\n", indices[i + 2] + 1, indices[i + 2] + 1,
           indices[i + 1] + 1, indices[i + 1] + 1, indices[i] + 1,
           indices[i] + 1);
  }
#endif
#endif
}

std::vector<float> lineData{};

// draw a line by dynamically storing vertex data
void draw_line(const glm::vec3 &p1, const glm::vec3 &p2,
               const glm::vec3 &color) {
  // point 1
  lineData.push_back(p1.x);
  lineData.push_back(p1.y);
  lineData.push_back(p1.z);

  // color
  lineData.push_back(color.r);
  lineData.push_back(color.g);
  lineData.push_back(color.b);

  // point 2
  lineData.push_back(p2.x);
  lineData.push_back(p2.y);
  lineData.push_back(p2.z);

  // color
  lineData.push_back(color.r);
  lineData.push_back(color.g);
  lineData.push_back(color.b);
}
void draw_line_2d(const glm::vec2 &p1, const glm::vec2 &p2,
                  const glm::vec3 &color) {
  draw_line(glm::vec3(p1, 0), glm::vec3(p2, 0), color);
}

void draw_lines_flush() {
  static unsigned int vao, vbo;

  static bool created = false;
  if (!created) {
    created = true;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, lineData.size() * sizeof(float),
                 lineData.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *)(3 * sizeof(float)));
  } else {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, lineData.size() * sizeof(float),
                 lineData.data(), GL_DYNAMIC_DRAW);
  }

  // 6 floats make up a vertex (3 position 3 color)
  // divide by that to get number of vertices to draw
  int count = lineData.size() / 6;

  glBindVertexArray(vao);
  glDrawArrays(GL_LINES, 0, count);

  lineData.clear();
}

// settings
const unsigned int SCR_WIDTH = 1366;
const unsigned int SCR_HEIGHT = 768;

void errorMessageCallback([[maybe_unused]] GLenum source,
                          [[maybe_unused]] GLenum type,
                          [[maybe_unused]] GLuint id,
                          [[maybe_unused]] GLenum severity,
                          [[maybe_unused]] GLsizei length,
                          const GLchar *message,
                          [[maybe_unused]] const void *userParam) {
  std::cout << "errorMessageCallback was called with message: " << message
            << std::endl;
}

namespace fs = std::filesystem;

int validateShaderFiles(const fs::path &projectPath, const fs::path &shaderDir,
                        const fs::path &vertexShaderFile,
                        const fs::path &fragmentShaderFile, Shader &shader) {
  auto vertexPath = projectPath / shaderDir / vertexShaderFile;
  auto fragmentPath = projectPath / shaderDir / fragmentShaderFile;
  if (!std::filesystem::exists(vertexPath) ||
      !std::filesystem::exists(fragmentPath)) {
    std::cout << "ERROR::SHADER::MODEL Failed to load shader files."
              << std::endl;
    std::cout << vertexPath.string() << std::endl;
    std::cout << fragmentPath.string() << std::endl;
    return -1;
  }
  std::string vertexFileName = vertexPath;
  std::string fragmentFileName = fragmentPath;
  if (shader.compile_and_link(vertexFileName.c_str(),
                              fragmentFileName.c_str())) {
    std::cout << "Shader compilation or linking error!\n";
    return -2;
  }
  return 0;
}

using entity = std::size_t;
entity MAX_ENTITY = 0;
entity create_entity() {
  static entity entities = 0;
  ++entities;
  MAX_ENTITY = entities;
  return entities;
}

struct transform_component {
  glm::vec3 pos{0.f};
  glm::vec3 vel{0.f};
  glm::vec3 rot{0.f};
  glm::vec3 scale{1.f};
  GLfloat rotationInDegrees = 0.f;
  GLfloat rotationvel = 0.f;
};

struct model_component {
  glm::mat4 modelMat{1.f};
  glm::mat4 modelDefaultOrientationRotation{1.f};
  Model &model;
  Material &material;
  Texture &texture;
  model_component(Model &model, Material &material, Texture &texture,
                  glm::mat4 defaultRotation = {1.f})
      : modelDefaultOrientationRotation{defaultRotation}, model{model},
        material{material}, texture{texture} {}
};

struct registry {
  std::unordered_map<entity, model_component> models;
  std::unordered_map<entity, transform_component> transforms;
};

struct model_system {

  void update(registry &reg) {
    for (std::size_t e = 1; e <= MAX_ENTITY; ++e) {
      if (reg.models.contains(e) && reg.transforms.contains(e)) {
        glm::mat4 &modelMatrix = reg.models.at(e).modelMat;
        modelMatrix = glm::mat4{1.f};
        auto &modelPosition = reg.transforms.at(e).pos;
        auto &scale = reg.transforms.at(e).scale;
        auto &rotationInDegrees = reg.transforms.at(e).rotationInDegrees;
        auto &rotation = reg.transforms.at(e).rot;
        auto &defaultrotation =
            reg.models.at(e).modelDefaultOrientationRotation;
        modelMatrix = glm::translate(modelMatrix, modelPosition);
        modelMatrix = glm::scale(modelMatrix, scale);
        // rotating the model by the loadoffset
        modelMatrix = modelMatrix * defaultrotation;
        modelMatrix =
            glm::rotate(modelMatrix, glm::radians(rotationInDegrees), rotation);
      }
    }
  }
  void render(registry &reg, Shader &shader) {
    for (std::size_t e = 1; e <= MAX_ENTITY; ++e) {
      if (reg.models.contains(e)) {
        auto &modelMatrix = reg.models.at(e).modelMat;
        auto &model = reg.models.at(e).model;
        auto &material = reg.models.at(e).material;
        auto &texture = reg.models.at(e).texture;
        shader.setMat4fv(modelMatrix, "model");
        shader.use();
        texture.useTexture();
        shader.useMaterial(material, "material.shininess",
                           "material.specularIntensity");
        model.renderModel(false);
        shader.unuse();
      }
    }
  }
};

struct transform_system {
  void update(registry &reg, float dt) {
    for (std::size_t e = 1; e <= MAX_ENTITY; ++e) {
      if (reg.transforms.contains(e)) {
        reg.transforms[e].pos += reg.transforms[e].vel * dt;
        reg.transforms[e].rotationInDegrees +=
            reg.transforms[e].rotationvel * dt;
      }
    }
  }
};

int main() {
  {
    Window window = Window(SCR_WIDTH, SCR_HEIGHT, GLFW_FALSE);
    window.Initialise();

    // create triangle
    meshList.reserve(3);
    createTriangle();

    /* glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1); */
    /* glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); */

    // -------------TUTORIAL-------------

    // OpenGL state
    // ------------
    /* glEnable(GL_CULL_FACE); */
    glEnable(GL_DEBUG_OUTPUT);
    glDisable(GL_BLEND);
    glDebugMessageCallback(errorMessageCallback, 0);
    glEnable(GL_DEPTH_TEST);

    // create camera
    Camera camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
                  -90.0f, 0.0f, 2.0f, 0.15f);

    // light source
    DirectionalLight mainLight =
        DirectionalLight({1.0f, 1.0f, 1.0f}, 0.3f, 0.4f, {0.0f, -1.0f, -1.0f});

    PointLight pointLights[MAX_POINT_LIGHTS];

    Spotlight spotLights[MAX_SPOT_LIGHTS];

    unsigned int pointLightCount = 0;
    unsigned int spotLightCount = 0;

    pointLights[0] = PointLight({0.0f, 0.0f, 1.0f}, 0.0f, 0.3f, 0.0f, 1.0f,
                                0.0f, 0.3f, 0.2f, 0.1f);
    /* pointLightCount++; */

    pointLights[1] = PointLight({0.0f, 1.0f, 0.0f}, 0.0f, 0.3f, -4.0f, 2.0f,
                                0.0f, 0.3f, 0.1f, 0.1f);

    /* pointLightCount++; */

    // angles in degrees
    float inner = 15.f;
    float outer = 30.f;

    spotLights[0] =
        Spotlight({1.0f, 1.0f, 1.0f}, 0.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
                  0.0f, 1.0f, 0.01f, 0.025f, 20.0f, cos(glm::radians(inner)),
                  cos(glm::radians(outer)));

    spotLightCount++;

    spotLights[1] =
        Spotlight({1.0f, 1.0f, 1.0f}, 0.0f, 1.0f, 5.0f, 0.0f, 0.0f, -100.0f,
                  -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 20.0f, inner, outer);

    /* spotLightCount++; */

    // material
    Material shinyMaterial = Material(4.0f, 256);
    Material dullMaterial = Material(0.3f, 2);

    // Check path
    auto executablePath = std::filesystem::current_path();
    auto projectPath = executablePath;
    std::string_view projectName = "H00M";
    while (projectPath != projectPath.root_path() &&
           projectPath.filename() != projectName) {
      projectPath = projectPath.parent_path();
      /* std::cout << projectPath.string() << std::endl; */
    }
    if (projectPath == projectPath.root_path()) {
      std::cout << "ERROR::FILESYSTEM: Failed to find project directory!"
                << std::endl;
      return -1;
    }
    // projectPath is valid from here on out

    Model cube;
    Model pyramid;
    Model teapot;
    Model sphere;
    try {
      cube.loadModel(projectPath / std::filesystem::path("cube-tex.obj"));
      pyramid.loadModel(projectPath / std::filesystem::path("pyramid2.obj"),
                        false);
      teapot.loadModel(projectPath / std::filesystem::path("teapot.obj"));
      sphere.loadModel(projectPath / std::filesystem::path("sphere.obj"));
    } catch (const std::invalid_argument &err) {
      std::cerr << err.what() << std::endl;
    }
    // load textures
    Texture brickTexture("../textures/brick.png");
    brickTexture.loadTextureAlpha();
    Texture dirtTexture("../textures/dirt.png");
    dirtTexture.loadTextureAlpha();
    Texture plainTexture("../textures/floor.png");
    plainTexture.loadTextureAlpha();

    // compile shaders
    Shader shader;
    Shader lineshader;
    // TODO: collect directories in one place
    auto shaderDir = std::filesystem::path("shaders");
    auto vertexShaderFile = std::filesystem::path("vertex_source.glsl.vert");
    auto fragmentShaderFile =
        std::filesystem::path("fragment_source.glsl.frag");
    if (validateShaderFiles(projectPath, shaderDir, vertexShaderFile,
                            fragmentShaderFile, shader)) {
      std::cout << "Error occured while validating shader files!\n";
      return -1;
    }
    auto lvf = std::filesystem::path("line.vert");
    auto lff = std::filesystem::path("line.frag");
    if (validateShaderFiles(projectPath, shaderDir, lvf, lff, lineshader)) {
      std::cout << "Error occured while validating shader files!\n";
      return -1;
    }

    glm::mat4 projection(1.f);
    projection =
        glm::perspective(45.0f,
                         (static_cast<GLfloat>(window.getBufferWidth()) /
                          static_cast<GLfloat>(window.getBufferHeight())),
                         0.1f, 100.0f);

    //------------------TEXT------------------
    TextRenderer textrenderer(SCR_WIDTH, SCR_HEIGHT);

    auto textVertexShaderFile = std::filesystem::path("text.vert");
    auto textFragmentShaderFile = std::filesystem::path("text.frag");
    auto textVertexPath = projectPath / shaderDir / textVertexShaderFile;
    auto textFragmentPath = projectPath / shaderDir / textFragmentShaderFile;
    if (!std::filesystem::exists(textVertexPath) ||
        !std::filesystem::exists(textFragmentPath)) {
      std::cout << "ERROR::SHADER::TEXT Failed to load shader files."
                << std::endl;
      std::cout << textVertexPath.string() << std::endl;
      std::cout << textFragmentPath.string() << std::endl;
      return -1;
    }
    //  find path to font
    auto fontDir = std::filesystem::path("fonts");
    auto fontFile = std::filesystem::path("Consolas-Bold.ttf");
    auto defaultFontPath = projectPath / fontDir / fontFile;
    if (!std::filesystem::exists(defaultFontPath)) {
      std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
      return -1;
    }

    if (textrenderer.init(textVertexPath, textFragmentPath, defaultFontPath)) {
      std::cout << "ERROR::TEXTRENDERER: Failed to init FontRenderer"
                << std::endl;
      return -1;
    }

    //------------------TEXT------------------

    model_system ms;
    transform_system ts;

    registry componentRegistry;

    // TODO: create component addition one-by-one instead
    auto addEntity = [&](Model &model, Material &mat, Texture &texture,
                         transform_component tc = {},
                         glm::mat4 defaultRotation = {1.f}) {
      if (MAX_ENTITY >= 20000) {
        std::cout << "Entity cap reached" << std::endl;
        return;
      }
      entity cubeEntity = create_entity();
      componentRegistry.models.emplace(
          cubeEntity, model_component{model, mat, texture, defaultRotation});
      componentRegistry.transforms[cubeEntity] = tc;
    };

    auto addEntities = [&](unsigned int count = 100) {
      if (MAX_ENTITY >= 20000) {
        std::cout << "Entity cap reached" << std::endl;
        return;
      }
      for (unsigned int i = 0; i < count; ++i) {
        entity cubeEntity = create_entity();
        componentRegistry.models.emplace(
            cubeEntity, model_component{cube, shinyMaterial, brickTexture});
        componentRegistry.transforms[cubeEntity] = transform_component{
            .pos = {0.f - rand() % 40, 10.f, -3.f - rand() % 40},
            .vel = {0.f, -0.1f - (float)(rand() % 10) / 10, 0.0f}};
      }
    };

    addEntity(sphere, shinyMaterial, brickTexture,
              transform_component{.pos = {-3.0f, -1.0f, -1.0f},
                                  .vel = glm::vec3{0.f},
                                  .rot = {0.0f, 1.0f, 0.0f},
                                  .scale = glm::vec3{1.f},
                                  .rotationInDegrees = 0.f,
                                  .rotationvel = 0.f});

    addEntity(cube, shinyMaterial, brickTexture,
              transform_component{.pos = {-2.0f, 1.0f, -3.0f},
                                  .vel = glm::vec3{0.f},
                                  .rot = {0.0f, 1.0f, 0.0f},
                                  .scale = glm::vec3{1.f},
                                  .rotationInDegrees = 0.f,
                                  .rotationvel = 0.f});

    {
      /* glm::mat4 pyramidRotationOffset{1.f}; */
      /* pyramidRotationOffset = */
      /*     glm::rotate(pyramidRotationOffset, glm::radians(-90.f), */
      /*                 glm::vec3{1.0f, 0.0f, 0.0f}); */
      addEntity(pyramid, shinyMaterial, brickTexture,
                transform_component{.pos = {0.0f, -1.0f, -3.0f},
                                    .vel = glm::vec3{0.f},
                                    .rot = {0.0f, 1.0f, 0.0f},
                                    .scale = glm::vec3{1.f},
                                    .rotationInDegrees = 0.f,
                                    .rotationvel = 10.f});

      addEntity(pyramid, dullMaterial, dirtTexture,
                transform_component{.pos = {0.0f, 1.0f, -3.0f},
                                    .vel = glm::vec3{0.f},
                                    .rot = {1.0f, 0.0f, 0.0f},
                                    .scale = glm::vec3{0.5f},
                                    .rotationInDegrees = 0.f,
                                    .rotationvel = 0.f});
    }

    while (!window.getShouldClose()) // returns true if window is closed
    {
      glEnable(GL_DEPTH_TEST);
      // 1 es 2 re vonalakra csereli a haromszogeket
      window.processInput(camera);

      // returns elapsed time in seconds as a double
      GLfloat now = glfwGetTime();
      deltaTime = now - lastTime;
      lastTime = now;

      // ----System update functions----
      ts.update(componentRegistry, deltaTime);
      ms.update(componentRegistry);
      // ----System update functions----

      camera.keyControl(window.getKeys(), deltaTime);
      camera.mouseControl(window.getXChange(), window.getYChange());

      if (camera.isFlashlightOn()) {
        spotLights[0].update(camera.getCameraPosition(),
                             camera.getCameraFront(), camera.getRight());
      } else {
        spotLights[0].disable();
      }

      // clear canvas
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      // render stuff

      //----Camera----
      glm::mat4 view = camera.calculateViewMatrix();
      // berakjuk a kamera helyet igy igazodik a visszaverodes
      shader.setVec3f(camera.getCameraPosition(), "eyePosition");
      shader.setMat4fv(view, "view");
      shader.setMat4fv(projection, "projection");
      //----Camera----

      //----Lighting data----
      shader.setDirectionalLight(mainLight);
      shader.setPointLights(pointLights, pointLightCount);
      shader.setSpotLights(spotLights, spotLightCount);
      //----Lighting data----

      // ----Ground----
      // render the ground (leaving it here for now bcs shadowmapping)
      glm::mat4 model = glm::mat4(1.f);
      model = glm::translate(model, glm::vec3(0.0f, -2.0005f, 0.0f));
      shader.setMat4fv(model, "model");
      shader.use();
      // beallitjuk a texturet
      dirtTexture.useTexture();
      shader.useMaterial(dullMaterial, "material.shininess",
                         "material.specularIntensity");
      // letrehozzuk a floort
      meshList[2].renderMesh();
      shader.unuse();
      // ----Ground----

      float length = 100.f;
      glm::vec3 linefrom = spotLights[0].getPos();
      glm::vec3 lineto = linefrom + spotLights[0].getDirection() * length;
      draw_line(linefrom, lineto, {1.f, 0.f, 0.f});
      lineshader.setMat4fv(view, "view");
      lineshader.setMat4fv(projection, "projection");
      lineshader.use();
      draw_lines_flush();

      // render all entities
      ms.render(componentRegistry, shader);
      // render all entities

      auto keys = window.getKeys();
      if (keys[GLFW_KEY_E]) {
        addEntities(1000);
        keys[GLFW_KEY_E] = false;
      }

      // handle performance debug output
      {
        if (now - lastTimeTextWasRendered > 0.1f) {
          lastTimeTextWasRendered = now;
          timeStr = "Elapsed time: " +
                    std::to_string(static_cast<unsigned int>(
                        std::floor(deltaTime * 1000.f))) +
                    " ms";
          FPSStr = "FPS: " + std::to_string(static_cast<unsigned int>(
                                 std::floor(1.f / deltaTime)));
          cameraLocStr = std::string("CAM location: ") +
                         glm::to_string(camera.getCameraPosition());
          cameraFacingStr = std::string("CAM facing: ") +
                            glm::to_string(camera.getCameraFront());
          cameraFacingStr =
              std::string("Entities: ") + std::to_string(MAX_ENTITY);
        }
        // ------ ADDING TEXT RENDER HERE ----------
        textrenderer.renderText(timeStr, 0.0f, SCR_HEIGHT - 24, 0.5f,
                                glm::vec3(1.0f, 1.0f, 1.0f));
        textrenderer.renderText(FPSStr, 0.0f, SCR_HEIGHT - 2 * 24, 0.5f,
                                glm::vec3(1.0f, 1.0f, 1.0f));
        textrenderer.renderText(cameraLocStr, 0.0f, SCR_HEIGHT - 3 * 24, 0.5f,
                                glm::vec3(1.0f, 1.0f, 1.0f));
        textrenderer.renderText(cameraFacingStr, 0.0f, SCR_HEIGHT - 4 * 24,
                                0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
        textrenderer.renderText(entityCountStr, 0.0f, SCR_HEIGHT - 4 * 24, 0.5f,
                                glm::vec3(1.0f, 1.0f, 1.0f));
        // ------ ADDING TEXT RENDER HERE ----------
      }

      // Print GL errors
      GLenum err;
      while ((err = glGetError()) != GL_NO_ERROR) {
        std::cout << "OpenGL error: " << err << std::endl;
      }

      // buffer swap(double buffering)
      window.swapBuffers();
      glfwPollEvents(); // special events
    }
  }
  glfwTerminate(); // terminates glfw library, cleanup

  return 0;
}
