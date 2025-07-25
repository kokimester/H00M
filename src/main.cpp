#define STB_IMAGE_IMPLEMENTATION
/* #include "stb_image.h" */

#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <print>
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
#include "Line.h"
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
constexpr GLfloat textTickRate = 10.0f;
std::string timeStr, FPSStr;
std::string cameraLocStr, cameraFacingStr, entityCountStr;

// settings
const unsigned int SCR_WIDTH = 1366;
const unsigned int SCR_HEIGHT = 768;
namespace fs = std::filesystem;

void errorMessageCallback([[maybe_unused]] GLenum source,
                          [[maybe_unused]] GLenum type,
                          [[maybe_unused]] GLuint id,
                          [[maybe_unused]] GLenum severity,
                          [[maybe_unused]] GLsizei length,
                          [[maybe_unused]] const GLchar *message,
                          [[maybe_unused]] const void *userParam)
{
  //std::println("errorMessageCallback was called with message: {}",message);
}


int validateShaderFiles(const fs::path &projectPath, const fs::path &shaderDir,
                        const fs::path &vertexShaderFile,
                        const fs::path &fragmentShaderFile, Shader &shader)
{
  auto vertexPath = projectPath / shaderDir / vertexShaderFile;
  auto fragmentPath = projectPath / shaderDir / fragmentShaderFile;
  if (!std::filesystem::exists(vertexPath) ||
      !std::filesystem::exists(fragmentPath))
  {
    std::cout << "ERROR::SHADER::MODEL Failed to load shader files."
              << std::endl;
    std::cout << vertexPath.string() << std::endl;
    std::cout << fragmentPath.string() << std::endl;
    return -1;
  }
  std::string vertexFileName = vertexPath.string();
  std::string fragmentFileName = fragmentPath.string();
  if (shader.compile_and_link(vertexFileName.c_str(),
                              fragmentFileName.c_str()))
  {
    std::cout << "Shader compilation or linking error!\n";
    return -2;
  }
  return 0;
}

using entity = std::size_t;
entity MAX_ENTITY = 0;
entity create_entity()
{
  static entity entities = 0;
  ++entities;
  MAX_ENTITY = entities;
  return entities;
}

struct transform_component
{
  glm::vec3 pos{0.f};
  glm::vec3 vel{0.f};
  glm::vec3 rot{0.f};
  glm::vec3 scale{1.f};
  GLfloat rotationInDegrees = 0.f;
  GLfloat rotationvel = 0.f;
};

struct model_component
{
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

struct registry
{
  std::unordered_map<entity, model_component> models;
  std::unordered_map<entity, transform_component> transforms;
};

struct model_system
{

  void update(registry &reg)
  {
    for (std::size_t e = 1; e <= MAX_ENTITY; ++e)
    {
      if (reg.models.contains(e) && reg.transforms.contains(e))
      {
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

  void render(registry &reg, Shader &shader)
  {
    for (std::size_t e = 1; e <= MAX_ENTITY; ++e)
    {
      if (reg.models.contains(e))
      {
        auto &modelMatrix = reg.models.at(e).modelMat;
        auto &model = reg.models.at(e).model;
        auto &material = reg.models.at(e).material;
        auto &texture = reg.models.at(e).texture;
        shader.setMat4fv(modelMatrix, "model");
        shader.use();
        texture.useTexture();
        shader.useMaterial(material, "material.shininess",
                           "material.specularIntensity");
        model.Render();
        shader.unuse();
      }
    }
  }
};

struct transform_system
{
  void update(registry &reg, float dt)
  {
    for (std::size_t e = 1; e <= MAX_ENTITY; ++e)
    {
      if (reg.transforms.contains(e))
      {
        reg.transforms[e].pos += reg.transforms[e].vel * dt;
        reg.transforms[e].rotationInDegrees +=
            reg.transforms[e].rotationvel * dt;
      }
    }
  }
};

// TODO: move this to a class
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
unsigned int depthMapFBO;
unsigned int depthMap;
void setupShadowTexture()
{
  glGenFramebuffers(1, &depthMapFBO);
  glGenTextures(1, &depthMap);
  glBindTexture(GL_TEXTURE_2D, depthMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH,
               SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         depthMap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// TODO: handle more light sources
void render(model_system &ms, registry &componentRegistry, Shader &shadow_shader, Shader &shader, const glm::mat4 &lightSpaceMatrix, unsigned int depthMapFBO, unsigned int depthMap)
{
  // 1. first render to depth map
  shadow_shader.setMat4fv(lightSpaceMatrix, "lightSpaceMatrix");
  glCullFace(GL_FRONT);
  glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glClear(GL_DEPTH_BUFFER_BIT);
  shadow_shader.use();
  ms.render(componentRegistry, shadow_shader);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glCullFace(GL_BACK); // don't forget to reset original culling face

  // 2. then render scene as normal with shadow mapping (using depth map)
  glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  shader.setMat4fv(lightSpaceMatrix, "lightSpaceMatrix");
  // brickTexture.useTexture();
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, depthMap);
  ms.render(componentRegistry, shader);
};

int loadshader(std::filesystem::path projectPath, const std::string &shaderDirStr, const std::string &shader_name, Shader &shader)
{
  std::string vertexFileExt = ".vert";
  std::string fragFileExt = ".frag";
  auto shaderDir = std::filesystem::path(shaderDirStr);
  auto vFile = std::filesystem::path(shader_name + vertexFileExt);
  auto fFile = std::filesystem::path(shader_name + fragFileExt);
  if (validateShaderFiles(projectPath, shaderDir, vFile,
                          fFile, shader))
  {
    std::cout << "Error occured while validating shader files!\n";
    return -1;
  }
  std::cout << "Succesfully built shader: " << shader_name << "\n";
  return 0;
};

bool isValidProjectPath(std::filesystem::path &projectPath)
{
  std::string_view projectName = "H00M";
  while (projectPath != projectPath.root_path() &&
         projectPath.filename() != projectName)
  {
    projectPath = projectPath.parent_path();
  }
  if (projectPath == projectPath.root_path())
  {
    std::cout << "ERROR::FILESYSTEM: Failed to find project directory!"
              << std::endl;
    return false;
  }
  return true;
}

unsigned int loadCubemap(const std::array<std::string_view, 6>& faces,const fs::path& skyBoxPath)
{
  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  int width, height, nrChannels;
  for (unsigned int i = 0; i < faces.size(); i++)
  {
    unsigned char *data = stbi_load(fs::path(skyBoxPath / faces[i]).string().c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                   0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    }
    else
    {
      std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
      stbi_image_free(data);
    }
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return textureID;
}

unsigned int loadSkybox(){

  float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    return skyboxVAO;
}

int main()
{
  {
    Window window = Window(SCR_WIDTH, SCR_HEIGHT, GLFW_FALSE);
    if(window.Initialise()){
      std::println("Window failed to initialize");
      return -1;
    }

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
        DirectionalLight({1.0f, 1.0f, 1.0f}, 0.5f, 0.7f, {0.0f, -1.0f, -1.0f});
    PointLight pointLights[MAX_POINT_LIGHTS];
    Spotlight spotLights[MAX_SPOT_LIGHTS];

    unsigned int pointLightCount = 0;
    unsigned int spotLightCount = 0;

    pointLights[0] = PointLight({1.0f, 1.0f, 1.0f}, 0.0f, 0.3f, 0.0f, 1.0f,
                                0.0f, 0.3f, 0.2f, 0.1f);
    pointLightCount++;

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

    // Check path
    auto projectPath = std::filesystem::current_path();
    if (!isValidProjectPath(projectPath))
    {
      std::cout << "Error occured while validating project path!\n";
      return -1;
    }

    // projectPath is valid from here on out
    Model cube;
    Model pyramid;
    Model teapot;
    Model sphere;
    Model ak;
    Model arm;
    fs::path modelDir = "models";
    try
    {
      // cube.loadModel(projectPath / modelDir / std::filesystem::path("cube-tex.obj"), true);
      // pyramid.loadModel(projectPath / modelDir / std::filesystem::path("pyramid2.obj"), true);
      // sphere.loadModel(projectPath / modelDir / std::filesystem::path("sphere.obj"), true);
      // ak.loadModel(projectPath / modelDir / std::filesystem::path("ak47.glb"), false);
      //arm.loadModel(projectPath / modelDir / std::filesystem::path("viewmodel.obj"), false);
    }
    catch (const std::invalid_argument &err)
    {
      std::cerr << err.what() << std::endl;
    }
    bool modelsLoaded = true;
    std::println("Loading cube");
    modelsLoaded &= cube.LoadMesh(projectPath / modelDir / std::filesystem::path("cube-tex.obj"));
    std::println("Loading pyramid");
    modelsLoaded &= pyramid.LoadMesh(projectPath / modelDir / std::filesystem::path("pyramid2.obj"));
    std::println("Loading sphere");
    modelsLoaded &= sphere.LoadMesh(projectPath / modelDir / std::filesystem::path("sphere.obj"));
    std::println("Loading viewmodel");
    modelsLoaded &= ak.LoadMesh(projectPath / modelDir / std::filesystem::path("ak47.gltf"));
    if(modelsLoaded == false){
      std::println("Models failed to load!");
      return -1;
    }
    std::cerr << "Loaded all models" << std::endl;
    // load textures
    Texture brickTexture("../textures/brick.png");
    brickTexture.loadTexture();
    Texture dirtTexture("../textures/dirt.png");
    dirtTexture.loadTexture();
    Texture plainTexture("../textures/floor.png");
    plainTexture.loadTexture();
    Texture whiteTexture("../textures/white.png");
    whiteTexture.loadTexture();
    std::cerr << "Loaded all textures" << std::endl;

    // material
    Material shinyMaterial = Material(4.0f, 256);
    Material dullMaterial = Material(0.3f, 2);

    // compile shaders
    Shader shader;
    Shader line_shader;
    Shader shadow_shader;
    Shader debugDepthQuad;
    Shader skyboxShader;
    Shader viewmodelShader;
    // TODO: collect directories in one place
    std::string shaderDir = "shaders";

    if (loadshader(projectPath, shaderDir, "main", shader))
      return -1;
    if (loadshader(projectPath, shaderDir, "line", line_shader))
      return -2;
    if (loadshader(projectPath, shaderDir, "depthShader", shadow_shader))
      return -3;
    if (loadshader(projectPath, shaderDir, "debugQuad", debugDepthQuad))
      return -4;
    if (loadshader(projectPath, shaderDir, "skybox", skyboxShader))
      return -5;
    if (loadshader(projectPath, shaderDir, "viewmodel", viewmodelShader))
      return -6;

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
        !std::filesystem::exists(textFragmentPath))
    {
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
    if (!std::filesystem::exists(defaultFontPath))
    {
      std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
      return -1;
    }

    if (textrenderer.init(textVertexPath, textFragmentPath, defaultFontPath))
    {
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
                         glm::mat4 defaultRotation = {1.f})
    {
      if (MAX_ENTITY >= 20000)
      {
        std::cout << "Entity cap reached" << std::endl;
        return;
      }
      std::println("Adding entity at: {} {} {}", tc.pos.x, tc.pos.y,tc.pos.z);
      entity cubeEntity = create_entity();
      componentRegistry.models.emplace(
          cubeEntity, model_component{model, mat, texture, defaultRotation});
      componentRegistry.transforms[cubeEntity] = tc;
    };

    auto addEntities = [&](unsigned int count = 100)
    {
      if (MAX_ENTITY >= 20000)
      {
        std::cout << "Entity cap reached" << std::endl;
        return;
      }
      for (unsigned int i = 0; i < count; ++i)
      {
        addEntity(cube, dullMaterial, brickTexture,
                  transform_component{
                      .pos = {(rand() % 40) - 20, 10.0f, (rand() % 40) - 20},
                      .vel = glm::vec3{0.f},
                      .rot = {0.0f, 1.0f, 0.0f},
                      .scale = glm::vec3{1.f},
                      .rotationInDegrees = 0.f,
                      .rotationvel = 0.f});
      }
    };

    // addEntity(ak, dullMaterial, brickTexture,
    //       transform_component{.pos = {0.0f, 1.0f, 0.0f},
    //                           .vel = glm::vec3{0.f},
    //                           .rot = {0.0f, 1.0f, 0.0f},
    //                           .scale = glm::vec3{0.1f},
    //                           .rotationInDegrees = 0.f,
    //                           .rotationvel = 0.f});

    addEntity(cube, dullMaterial, brickTexture,
              transform_component{.pos = {-10.0f, -2.0005f, -10.0f},
                                  .vel = glm::vec3{0.f},
                                  .rot = {0.0f, 1.0f, 0.0f},
                                  .scale = glm::vec3{20.f, 0.01f, 20.f},
                                  .rotationInDegrees = 0.f,
                                  .rotationvel = 0.f});

    addEntity(sphere, dullMaterial, brickTexture,
              transform_component{.pos = {-3.0f, 1.0f, -1.0f},
                                  .vel = glm::vec3{0.f},
                                  .rot = {0.0f, 1.0f, 0.0f},
                                  .scale = glm::vec3{1.f},
                                  .rotationInDegrees = 0.f,
                                  .rotationvel = 0.f});

    addEntity(cube, dullMaterial, brickTexture,
              transform_component{.pos = {-2.0f, 1.0f, -3.0f},
                                  .vel = glm::vec3{0.f},
                                  .rot = {0.0f, 1.0f, 0.0f},
                                  .scale = glm::vec3{1.f},
                                  .rotationInDegrees = 0.f,
                                  .rotationvel = 0.f});

    {
      addEntity(pyramid, shinyMaterial, brickTexture,
                transform_component{.pos = {0.0f, -1.0f, -3.0f},
                                    .vel = glm::vec3{0.f},
                                    .rot = {0.0f, 1.0f, 0.0f},
                                    .scale = glm::vec3{1.f},
                                    .rotationInDegrees = 0.f,
                                    .rotationvel = 10.f});

      addEntity(pyramid, dullMaterial, dirtTexture,
                transform_component{.pos = {0.0f, 2.0f, -3.0f},
                                    .vel = glm::vec3{0.f},
                                    .rot = {1.0f, 0.0f, 0.0f},
                                    .scale = glm::vec3{0.5f},
                                    .rotationInDegrees = 0.f,
                                    .rotationvel = 0.f});
    }

    shader.set1i(0, "diffuseTexture");
    shader.set1i(1, "shadowMap");
    
    viewmodelShader.set1i(0, "diffuseTexture");

    debugDepthQuad.set1i(0, "depthMap");
    skyboxShader.set1i(0,"skybox");

    viewmodelShader.setupBones();

    setupShadowTexture();
    // setup cubemap
    fs::path textureDir("textures");
    fs::path skyboxDir("skybox");
    fs::path skyBoxPath = projectPath / textureDir / skyboxDir;
    std::array<std::string_view, 6> cubemapFaces =
    {
      "right.jpg",
      "left.jpg",
      "top.jpg",
      "bottom.jpg",
      "front.jpg",
      "back.jpg"};
      
    unsigned int cubemapTexture = loadCubemap(cubemapFaces,skyBoxPath);
    
    unsigned int skyboxVAO = loadSkybox();
    GLfloat startTime = glfwGetTime();
    GLint activeBoneIndex = 0;

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

      if (camera.isFlashlightOn())
      {
        spotLights[0].update(camera.getCameraPosition(),
                             camera.getCameraFront(), camera.getRight());
      }
      else
      {
        //spotLights[0].disable();
        auto pos = glm::vec3(glm::cos(now / 2) * 5.f, 10.0f, glm::sin(now / 2) * 5.f);
        spotLights[0].update(pos,
                             glm::normalize(glm::vec3(0.f) - pos), camera.getRight());
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
      //----Lighting data----
      viewmodelShader.setDirectionalLight(mainLight);
      viewmodelShader.setPointLights(pointLights, pointLightCount);
      viewmodelShader.setSpotLights(spotLights, spotLightCount);
      //----Lighting data----

      // ----Shadow pass-----
      float near_plane = 1.0f, far_plane = 25.f;

      glm::mat4 lightProjection =
          glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
      glm::mat4 lightView = glm::lookAt(-10.f * mainLight.getDirection(),
                                        glm::vec3(0.0f, 0.0f, 0.0f),
                                        glm::vec3(0.0f, 1.0f, 0.0f));
      auto &flashlight = spotLights[0];
      lightView = glm::lookAt(flashlight.getPos(), flashlight.getPos() + flashlight.getDirection(),
                              glm::vec3(0.f, 1.f, 0.f));
      glm::mat4 lightSpaceMatrix = lightProjection * lightView;

      render(ms, componentRegistry, shadow_shader, shader, lightSpaceMatrix, depthMapFBO, depthMap);

      // debug quad
      //  debugDepthQuad.use();
      //  debugDepthQuad.set1f(near_plane,"near_plane");
      //  debugDepthQuad.set1f(far_plane, "far_plane");
      //  glActiveTexture(GL_TEXTURE0);
      //  glBindTexture(GL_TEXTURE_2D, depthMap);
      // debug quad

      static Line line;
      line.updateWithDirection(spotLights[0].getPos(),
                               spotLights[0].getDirection(), {1.f, 0.f, 0.f});
      line_shader.setMat4fv(view, "view");
      line_shader.setMat4fv(projection, "projection");
      line_shader.use();
      line.render();

      // render all entities
      /* ms.render(componentRegistry, shader); */
      // render all entities

      // ----Lighting pass-----

      auto keys = window.getKeys();
      if (keys[GLFW_KEY_E])
      {
        //addEntities(1000);
        activeBoneIndex = (activeBoneIndex + 1) % ak.NumBones();
        std::println("Active bone: {}", activeBoneIndex);
        keys[GLFW_KEY_E] = false;
      }
      static bool currentlyInAnimation = false;
      static float maxAnimationTime = 1.f;
      //reloading
      if (keys[GLFW_KEY_R])
      {
        ak.SetActiveAnimation(0);
        if(!currentlyInAnimation){
          startTime = now;
          currentlyInAnimation = true;
          maxAnimationTime = 1.f;
        }
        keys[GLFW_KEY_R] = false;
      }
      //shooting
      pointLights[0].setPos(camera.getCameraPosition()+0.5f * camera.getCameraFront());
      if (keys[GLFW_KEY_Q])
      {
        if(!currentlyInAnimation){
          ak.SetActiveAnimation(2);
          startTime = now;
          currentlyInAnimation = true;
          maxAnimationTime = 0.25f;
          pointLightCount = 1;
          auto plPos = pointLights[0].getPos();
          std::println("Adding muzzle flash at: {} {} {}",plPos.x,plPos.y,plPos.z);
        }
      }
      float AnimationTimeSec = 0.f;
      if(currentlyInAnimation){
        AnimationTimeSec = now - startTime;
      }else{
        AnimationTimeSec = 0.f;
        pointLightCount = 0;
      }
      if(AnimationTimeSec > maxAnimationTime){
        currentlyInAnimation = false;
      }

      


      //----Skybox----
      // draw skybox as last
      glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
      auto skyboxView = glm::mat4(glm::mat3(view));
      skyboxShader.setMat4fv(skyboxView, "view");
      skyboxShader.setMat4fv(projection, "projection");
      // ... set view and projection matrix
      skyboxShader.use();
      glBindVertexArray(skyboxVAO);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
      glDrawArrays(GL_TRIANGLES, 0, 36);
      glDepthMask(GL_TRUE);
      glBindVertexArray(0);
      glDepthFunc(GL_LESS); // set depth function back to default
      //----Skybox----

      //----Viewmodel rendering-----
      glClear(GL_DEPTH_BUFFER_BIT);
      //glDisable(GL_DEPTH_TEST);
      auto modelMatrix = glm::mat4{1.f};
      auto scale = glm::vec3{0.1f};
      modelMatrix = glm::translate(modelMatrix, glm::vec3{0.f,0.f,0.f});
      modelMatrix = glm::scale(modelMatrix, scale);
      modelMatrix = glm::rotate(modelMatrix, glm::radians(-180.f), glm::vec3{0.f,1.f,0.f});
      viewmodelShader.setMat4fv(modelMatrix,"model");
      //viewmodelShader.setMat4fv(glm::mat4(1.f),"view");
      viewmodelShader.setMat4fv(view,"view");
      viewmodelShader.setMat4fv(projection, "projection");
      viewmodelShader.set1i(activeBoneIndex,"activeBoneIndex");
      viewmodelShader.use();
      viewmodelShader.useMaterial(shinyMaterial, "material.shininess",
                            "material.specularIntensity");
      std::vector<glm::mat4> boneTransforms;
      //static float AnimationTimeSec = 0;
      ak.GetBoneTransforms(AnimationTimeSec, boneTransforms);
      for (size_t i = 1 ; i < boneTransforms.size() ; i++) {
        //boneTransforms[i] = glm::mat4{1.f};
        viewmodelShader.setBoneTransform(i, boneTransforms[i]);
      }
      viewmodelShader.use();
      //brickTexture.useTexture();
      ak.Render();
      //glEnable(GL_DEPTH_TEST);
      //----Viewmodel rendering-----

      // handle performance debug output
      {
        if (now - lastTimeTextWasRendered > 1 / textTickRate)
        {
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
          entityCountStr =
              std::string("Entities: ") + std::to_string(MAX_ENTITY);
        }
        //TODO: Fix text render, currently not drawing on the skybox
        // ------ ADDING TEXT RENDER HERE ----------
        textrenderer.renderText(timeStr, 0.0f, SCR_HEIGHT - 24, 0.5f,
                                glm::vec3(1.0f, 1.0f, 1.0f));
        textrenderer.renderText(FPSStr, 0.0f, SCR_HEIGHT - 2 * 24, 0.5f,
                                glm::vec3(1.0f, 1.0f, 1.0f));
        textrenderer.renderText(cameraLocStr, 0.0f, SCR_HEIGHT - 3 * 24, 0.5f,
                                glm::vec3(1.0f, 1.0f, 1.0f));
        textrenderer.renderText(cameraFacingStr, 0.0f, SCR_HEIGHT - 4 * 24,
                                0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
        textrenderer.renderText(entityCountStr, 0.0f, SCR_HEIGHT - 5 * 24, 0.5f,
                                glm::vec3(1.0f, 1.0f, 1.0f));
        // ------ ADDING TEXT RENDER HERE ----------
      }
      

      // buffer swap(double buffering)
      window.swapBuffers();
      glfwPollEvents(); // special events
    }
  }
  glfwTerminate(); // terminates glfw library, cleanup

  return 0;
}
