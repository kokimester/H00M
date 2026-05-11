#define STB_IMAGE_IMPLEMENTATION

#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <print>
#include <string>

#include <cmath>
// TODO: add parallelization for loading assets and shaders
#include <future>
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
#include "Entity.h"
#include "Line.h"
#include "Material.h"
#include "Mesh.h"
#include "Model.h"
#include "Player.h"
#include "PointLight.h"
#include "Shader.h"
#include "ShadowMap.h"
#include "Skybox.h"
#include "Spotlight.h"
#include "Text.h"
#include "Texture.h"
#include "Utility.h"
#include "Window.h"

#include <assimp/Importer.hpp>

#ifdef __linux__
#include <sys/inotify.h>
#elif _WIN32
#else
#error "Not supported OS"
#endif

// TODO: Globals, move somewhere else, as static perhaps

GLfloat deltaTime               = 0.0f;
GLfloat lastTime                = 0.0f;
GLfloat lastTimeTextWasRendered = 0.0f;
constexpr GLfloat textTickRate  = 10.0f;
std::string timeStr, FPSStr;
std::string cameraLocStr, cameraFacingStr, entityCountStr, playerPositionStr,
    playerVelocityStr;

// settings
namespace fs = std::filesystem;

void errorMessageCallback([[maybe_unused]] GLenum source,
                          [[maybe_unused]] GLenum type,
                          [[maybe_unused]] GLuint id,
                          [[maybe_unused]] GLenum severity,
                          [[maybe_unused]] GLsizei length,
                          [[maybe_unused]] const GLchar* message,
                          [[maybe_unused]] const void* userParam) {
  // std::println("errorMessageCallback was called with message: {}",message);
}
// TODO: handle more light sources
int main() {
  {
    Window window = Window(SCR_WIDTH, SCR_HEIGHT, GLFW_FALSE);
    if (window.Initialise()) {
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
        DirectionalLight({1.0f, 1.0f, 1.0f}, 0.5f, 0.7f, {0.0f, -0.3f, -1.0f});
    PointLight pointLights[MAX_POINT_LIGHTS];
    Spotlight spotLights[MAX_SPOT_LIGHTS];

    unsigned int pointLightCount = 0;
    unsigned int spotLightCount  = 0;

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
    auto projectPath = fs::current_path();
    if (!isValidProjectPath(projectPath)) {
      std::println(stderr, "Error occured while validating project path!");
      return -1;
    }

    // projectPath is valid from here on out
    Model cube;
    Model pyramid;
    Model teapot;
    Model sphere;
    Model ak;
    Model arm;
    Model capsule;
    fs::path modelDir = "models";

    // assett loading multithreaded:
    // https://www.reddit.com/r/opengl/comments/17httnd/help_with_loading_assets_with_multithreading/
    auto loadModelsLambda = [&]() {
      std::println("Loading models");
      auto modelLoadStartTime = glfwGetTime();

      bool modelsLoaded = true;
      std::println("Loading cube");
      modelsLoaded &=
          cube.LoadMesh(projectPath / modelDir / fs::path("cube-tex.obj"));
      std::println("Loading pyramid");
      modelsLoaded &=
          pyramid.LoadMesh(projectPath / modelDir / fs::path("pyramid2.obj"));
      std::println("Loading sphere");
      modelsLoaded &=
          sphere.LoadMesh(projectPath / modelDir / fs::path("sphere.obj"));
      std::println("Loading viewmodel");
      modelsLoaded &=
          ak.LoadMesh(projectPath / modelDir / fs::path("ak47.gltf"));
      std::println("Loading capsule");
      modelsLoaded &= capsule.LoadMesh(projectPath / modelDir /
                                       fs::path("capsule_smooth.obj"));
      if (modelsLoaded == false) {
        std::println("Models failed to load!");
        // return -1;
        return false;
      }

      std::println("Loaded models in {} ms.",
                   (glfwGetTime() - modelLoadStartTime) * 1000.f);
      return true;
    };

    loadModelsLambda();

    std::println("Loading textures");
    auto texturesLoadStartTime = glfwGetTime();
    // load textures
    Texture brickTexture("../textures/brick.png");
    brickTexture.loadTexture();
    Texture dirtTexture("../textures/dirt.png");
    dirtTexture.loadTexture();
    Texture plainTexture("../textures/floor.png");
    plainTexture.loadTexture();
    Texture whiteTexture("../textures/white.png");
    whiteTexture.loadTexture();
    Texture muzzleFlashTexture("../textures/muzzleflash.png");
    muzzleFlashTexture.loadTexture();
    std::println("Loaded textures in {} ms.",
                 (glfwGetTime() - texturesLoadStartTime) * 1000.f);

    // material
    Material shinyMaterial = Material(4.0f, 256);
    Material dullMaterial  = Material(0.3f, 2);

    // compile shaders
    Shader shader;
    Shader lineShader;
    Shader boxShader;
    Shader shadowShader;
    Shader debugDepthQuad;
    Shader skyboxShader;
    Shader viewmodelShader;
    Shader muzzleFlashShader;
    // TODO: collect directories in one place
    std::string shaderDir = "shaders";
    std::println("Loading shaders");
    using ShaderInfo         = std::pair<std::string_view, Shader&>;
    const std::array shaders = {ShaderInfo{"main", shader},
                                ShaderInfo{"line", lineShader},
                                ShaderInfo{"box", boxShader},
                                ShaderInfo{"shadowShader", shadowShader},
                                ShaderInfo{"debugQuad", debugDepthQuad},
                                ShaderInfo{"skybox", skyboxShader},
                                ShaderInfo{"viewmodel", viewmodelShader},
                                ShaderInfo{"muzzle", muzzleFlashShader}};
    auto shaderLoadStartTime = glfwGetTime();

    // Loading shaders
    // Scope to not pollute namespace with count variable
    // TODO: add custom lambda function for shaders to handle hot-reloading
    // NOTE: e.g. for main shader the following needs to be set:
    // shader.set1i(0, "diffuseTexture");
    // shader.set1i(1, "shadowMap");
    //
    {
      int count = 0;
      for (auto& shaderItr : shaders) {
        ++count;
        if (loadshader(projectPath, shaderDir, shaderItr.first,
                       shaderItr.second))
          return -count;
      }
    }
    std::println("Loaded shaders in {} ms.",
                 (glfwGetTime() - shaderLoadStartTime) * 1000.f);

    // shader file observers
    std::vector<std::unique_ptr<FileObserver>> observers;
    for (auto& shaderItr : shaders) {
      auto vertFile = std::string{shaderItr.first} + std::string{".vert"};
      auto fragFile = std::string{shaderItr.first} + std::string{".frag"};
      observers.emplace_back(fileObserverFactory());
      observers.back()->add_watch(projectPath / shaderDir / vertFile);
      observers.back()->add_watch(projectPath / shaderDir / fragFile);
      observers.back()->subscribe(&shader);
    }
    // shader file observers

    glm::mat4 projection(1.f);
    projection =
        glm::perspective(45.0f,
                         (static_cast<GLfloat>(window.getBufferWidth()) /
                          static_cast<GLfloat>(window.getBufferHeight())),
                         0.1f, 100.0f);

    //------------------TEXT------------------
    // TODO: add shader from the outside to allow hot-reloading
    TextRenderer textrenderer(SCR_WIDTH, SCR_HEIGHT);

    auto textVertexShaderFile   = fs::path("text.vert");
    auto textFragmentShaderFile = fs::path("text.frag");
    auto textVertexPath   = projectPath / shaderDir / textVertexShaderFile;
    auto textFragmentPath = projectPath / shaderDir / textFragmentShaderFile;
    if (!fs::exists(textVertexPath) || !fs::exists(textFragmentPath)) {
      std::cout << "ERROR::SHADER::TEXT Failed to load shader files."
                << std::endl;
      std::cout << textVertexPath.string() << std::endl;
      std::cout << textFragmentPath.string() << std::endl;
      return -1;
    }
    //  find path to font
    auto fontDir         = fs::path("fonts");
    auto fontFile        = fs::path("Consolas-Bold.ttf");
    auto defaultFontPath = projectPath / fontDir / fontFile;
    if (!fs::exists(defaultFontPath)) {
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
    hitbox_system hs;

    registry componentRegistry;

    // TODO: create component addition one-by-one instead
    auto addEntity = [&](Model& model, Material& mat, Texture& texture,
                         transform_component tc    = {},
                         glm::mat4 defaultRotation = {1.f}) -> EntityType {
      if (MAX_ENTITY >= 20000) {
        std::println("Entity cap reached");
        // this prevents an invalid return statement
        // and enables error handling
        throw std::runtime_error("Entity cap reached");
      }
      auto entityID = create_entity();
      std::println("[{}] Adding entity at: {} {} {}", entityID, tc.pos.x,
                   tc.pos.y, tc.pos.z);
      componentRegistry.models.emplace(
          entityID, model_component{model, mat, texture, defaultRotation});
      componentRegistry.transforms[entityID] = tc;
      return entityID;
    };

    /*
    auto addEntities = [&](unsigned int count = 100) {
      if (MAX_ENTITY >= 20000) {
        std::cout << "Entity cap reached" << std::endl;
        return;
      }
      for (unsigned int i = 0; i < count; ++i) {
        addEntity(cube, dullMaterial, brickTexture,
                  transform_component{
                      .pos   = {(rand() % 40) - 20, 10.0f, (rand() % 40) - 20},
                      .vel   = glm::vec3{0.f},
                      .rot   = {0.0f, 1.0f, 0.0f},
                      .scale = glm::vec3{1.f},
                      .rotationInDegrees = 0.f,
                      .rotationvel       = 0.f});
      }
    };
    */
    // add player entity, transform_component only

    auto playerEntityID     = create_entity();
    auto& playerCamera      = camera;
    auto& playerRegistryRef = componentRegistry;

    componentRegistry.transforms[playerEntityID] = transform_component{

        .pos               = glm::vec3{0.f, 2.f, 0.f},
        .vel               = glm::vec3{0.f},
        .acc               = glm::vec3{0.f, -10.f, 0.f},
        .rot               = {0.0f, 1.0f, 0.0f},
        .scale             = glm::vec3{1.f},
        .rotationInDegrees = 0.f,
        .rotationvel       = 0.f

    };

    auto playerPos = componentRegistry.transforms[playerEntityID].pos;
    componentRegistry.collision_boxes[playerEntityID] =
        collision_component{.box = {playerPos + glm::vec3{-0.5f, 0.f, -0.5f},
                                    playerPos + glm::vec3{0.5f, 1.0f, 0.5f}},
                            .isMovable = true};

    auto player = Player(playerEntityID, playerRegistryRef, playerCamera);

    // floor
    auto floorEntityID =
        addEntity(cube, dullMaterial, brickTexture,
                  transform_component{.pos   = {-10.0f, -2.0f, -10.0f},
                                      .vel   = glm::vec3{0.f},
                                      .rot   = {0.0f, 1.0f, 0.0f},
                                      .scale = glm::vec3{20.f, 0.01f, 20.f},
                                      .rotationInDegrees = 0.f,
                                      .rotationvel       = 0.f});

    componentRegistry.collision_boxes[floorEntityID] =
        collision_component{.box       = {glm::vec3{-10.0f, -3.0f, -10.0f},
                                          glm::vec3{10.0f, -2.0f, 10.0f}},
                            .isMovable = false};

    addEntity(sphere, dullMaterial, brickTexture,
              transform_component{.pos               = {-3.0f, 1.0f, -1.0f},
                                  .vel               = glm::vec3{0.f},
                                  .rot               = {0.0f, 1.0f, 0.0f},
                                  .scale             = glm::vec3{1.f},
                                  .rotationInDegrees = 0.f,
                                  .rotationvel       = 0.f});

    auto boxEntityID =
        addEntity(cube, dullMaterial, brickTexture,
                  transform_component{.pos   = {-2.0f, -2.0f, -1.0f},
                                      .vel   = glm::vec3{0.f},
                                      .rot   = {0.0f, 1.0f, 0.0f},
                                      .scale = glm::vec3{1.f},
                                      .rotationInDegrees = 0.f,
                                      .rotationvel       = 0.f});

    componentRegistry.collision_boxes[boxEntityID] = collision_component{
        .box = {glm::vec3{-2.0f, -2.0f, -1.0f}, glm::vec3{-1.0f, -1.0f, 0.0f}},
        .isMovable = false};

    addEntity(pyramid, shinyMaterial, brickTexture,
              transform_component{.pos               = {0.0f, -1.0f, -3.0f},
                                  .vel               = glm::vec3{0.f},
                                  .rot               = {0.0f, 1.0f, 0.0f},
                                  .scale             = glm::vec3{1.f},
                                  .rotationInDegrees = 0.f,
                                  .rotationvel       = 10.f});

    addEntity(pyramid, dullMaterial, dirtTexture,
              transform_component{.pos               = {0.0f, 2.0f, -3.0f},
                                  .vel               = glm::vec3{0.f},
                                  .rot               = {1.0f, 0.0f, 0.0f},
                                  .scale             = glm::vec3{0.5f},
                                  .rotationInDegrees = 0.f,
                                  .rotationvel       = 0.f});

    auto enemyID =
        addEntity(capsule, dullMaterial, whiteTexture,
                  transform_component{.pos   = {0.0f, 0.0f, 3.0f},
                                      .vel   = glm::vec3{0.f},
                                      .acc   = glm::vec3{0.f, -1.f, 0.f},
                                      .rot   = {1.0f, 0.0f, 0.0f},
                                      .scale = glm::vec3{1.0f, 0.9f, 1.0f},
                                      .rotationInDegrees = 0.f,
                                      .rotationvel       = 0.f});
    componentRegistry.collision_boxes[enemyID] =
        collision_component{.box       = {glm::vec3{-0.5f, 0.0f, 3.f - 0.5f},
                                          glm::vec3{0.5f, 1.5f, 3.f + 0.5f}},
                            .isMovable = true};

    componentRegistry.hitboxes[enemyID] =
        hitbox_component{.box        = {glm::vec3{-0.3f, 0.0f, 3.f - 0.3f},
                                        glm::vec3{0.3f, 1.5f, 3.f + 0.3f}},
                         .isTargeted = false};

    // TODO: set the numbers based on a common header file
    shader.set1i(0, "diffuseTexture");
    shader.set1i(1, "shadowMap");

    viewmodelShader.set1i(0, "diffuseTexture");

    muzzleFlashShader.set1i(0, "diffuseTexture");

    debugDepthQuad.set1i(0, "depthMap");
    skyboxShader.set1i(0, "skybox");

    viewmodelShader.setupBones();

    ShadowMap shadowmap;
    shadowmap.setup();

    // setup cubemap for skybox
    fs::path textureDir("textures");
    fs::path skyboxDir("skybox");
    fs::path skyBoxPath = projectPath / textureDir / skyboxDir;
    std::array<std::string_view, 6> cubemapFaces = {"right.jpg", "left.jpg",
                                                    "top.jpg",   "bottom.jpg",
                                                    "front.jpg", "back.jpg"};
    Skybox skybox(skyboxShader, skyBoxPath, cubemapFaces);
    // create muzzleflash mesh

    std::vector<unsigned int> muzzleIndices{0, 1, 2, 1, 2, 3};
    std::vector<GLfloat> muzzleVertices{

        -1.0f, 0.0f, -1.f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        1.0f,  0.0f, -1.f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        -1.0f, 0.0f, 1.f,  0.0f, 1.0f, 0.0f, -1.0f, 0.0f,

        1.0f,  0.0f, 1.f,  1.0f, 1.0f, 0.0f, -1.0f, 0.0f};

    Mesh muzzleFlashMesh;
    muzzleFlashMesh.createMesh(muzzleVertices, muzzleIndices);

    // need this here to not get a crazy first frame
    // i was falling under the ground because of a large deltaTime
    lastTime = glfwGetTime();
    while (!window.getShouldClose()) // returns true if window is closed
    {
      glEnable(GL_DEPTH_TEST);
      // 1 es 2 re vonalakra csereli a haromszogeket
      // window.processInput(camera);

      // returns elapsed time in seconds as a double
      GLfloat now = glfwGetTime();
      deltaTime   = now - lastTime;
      lastTime    = now;

      // ----System update functions----
      ts.update(componentRegistry, deltaTime);
      ms.update(componentRegistry);
      Line shootRay;
      shootRay.updateWithDirection(player.getCamera().getCameraPosition(),
                                   player.getCamera().getCameraFront(),
                                   glm::vec3{1.f});
      hs.update(componentRegistry, shootRay);

      // ----System update functions----

      /* camera.keyControl(window.getKeys(), deltaTime); */
      /* camera.mouseControl(window.getXChange(), window.getYChange()); */

      player.handleKeyboardInput(window.getKeys(), deltaTime);
      player.handleMouseInput(window.getXChange(), window.getYChange());
      player.update(deltaTime);

      if (true || camera.isFlashlightOn()) {
        spotLights[0].update(camera.getCameraPosition(),
                             camera.getCameraFront(), camera.getRight());
      } else {
        // spotLights[0].disable();
        auto pos =
            glm::vec3(glm::cos(now / 2) * 5.f, 10.0f, glm::sin(now / 2) * 5.f);
        spotLights[0].update(pos, glm::normalize(glm::vec3(0.f) - pos),
                             camera.getRight());
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
      glm::mat4 lightView =
          glm::lookAt(-10.f * mainLight.getDirection(),
                      glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
      // auto& flashlight           = spotLights[0];
      // lightView                  = glm::lookAt(flashlight.getPos(),
      //                                          flashlight.getPos() +
      //                                          flashlight.getDirection(),
      //                                          glm::vec3(0.f, 1.f, 0.f));
      glm::mat4 lightSpaceMatrix = lightProjection * lightView;

      shadowmap.render(ms, componentRegistry, shadowShader, shader,
                       lightSpaceMatrix);

      // debug quad
      //  debugDepthQuad.use();
      //  debugDepthQuad.set1f(near_plane,"near_plane");
      //  debugDepthQuad.set1f(far_plane, "far_plane");
      //  glActiveTexture(GL_TEXTURE0);
      //  glBindTexture(GL_TEXTURE_2D, depthMap);
      // debug quad

      // ----Lighting pass-----

      auto keys = window.getKeys();
      if (keys[GLFW_KEY_E]) {
        // addEntities(1000);
        keys[GLFW_KEY_E] = false;
      }
      // reloading
      if (keys[GLFW_KEY_R]) {
        ak.SetActiveAnimation("reload");
        keys[GLFW_KEY_R] = false;
      }
      // shooting
      pointLights[0].setPos(camera.getCameraPosition() +
                            0.5f * camera.getCameraFront());

      if (keys[GLFW_KEY_Q]) {
        ak.SetActiveAnimation("shoot");
        pointLightCount = 1;
      }
      if (ak.GetAnimationState() == Model::ANIMATION_STATE::IDLE) {
        pointLightCount = 0;
      }

      //----Skybox----
      // draw skybox as last
      auto skyboxView = glm::mat4(glm::mat3(view));
      skyboxShader.setMat4fv(skyboxView, "view");
      skyboxShader.setMat4fv(projection, "projection");
      skybox.render();
      //----Skybox----

      static Line line;
      line.updateWithDirection(spotLights[0].getPos(),
                               spotLights[0].getDirection(), {1.f, 0.f, 0.f});
      lineShader.setMat4fv(view, "view");
      lineShader.setMat4fv(projection, "projection");
      lineShader.use();
      line.render();

      boxShader.setMat4fv(view, "view");
      boxShader.setMat4fv(projection, "projection");
      boxShader.use();
      // renderBoxes(componentRegistry.collision_boxes);
      // renderBoxes(componentRegistry.hitboxes);

      //--Render muzzleflash--
      if (ak.GetActiveAnimationName() == "shoot") {
        if (ak.GetAnimationState() == Model::ANIMATION_STATE::IN_ANIMATION) {
          auto muzzleTranslation = glm::mat4{1.f};
          muzzleTranslation =
              glm::translate(muzzleTranslation, glm::vec3{0.3f, -0.25f, -2.2f});
          muzzleTranslation = glm::rotate(muzzleTranslation, glm::radians(90.f),
                                          glm::vec3{1.f, 0.f, 0.f});
          muzzleTranslation = glm::scale(muzzleTranslation, glm::vec3{0.3f});
          muzzleFlashShader.setMat4fv(projection, "projection");
          muzzleFlashShader.setMat4fv(view, "view");
          muzzleFlashShader.setMat4fv(muzzleTranslation, "model");
          muzzleFlashShader.set1f(ak.GetAnimationProgress(), "progress");

          muzzleFlashShader.use();
          glEnable(GL_BLEND);
          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
          muzzleFlashTexture.useTexture();
          muzzleFlashShader.use();
          muzzleFlashMesh.renderMesh();

          glDisable(GL_BLEND);
        }
      }
      //--Render muzzleflash--

      //----Viewmodel rendering-----
      glClear(GL_DEPTH_BUFFER_BIT);
      auto modelMatrix = glm::mat4{1.f};
      auto scale       = glm::vec3{0.1f};
      modelMatrix      = glm::translate(modelMatrix, glm::vec3{0.f, 0.f, 0.f});
      modelMatrix      = glm::scale(modelMatrix, scale);
      modelMatrix      = glm::rotate(modelMatrix, glm::radians(-180.f),
                                     glm::vec3{0.f, 1.f, 0.f});
      viewmodelShader.setMat4fv(modelMatrix, "model");
      // viewmodelShader.setMat4fv(glm::mat4(1.f),"view");
      viewmodelShader.setMat4fv(view, "view");
      viewmodelShader.setMat4fv(projection, "projection");
      viewmodelShader.use();
      viewmodelShader.useMaterial(shinyMaterial, "material.shininess",
                                  "material.specularIntensity");
      // set player translation for fragpos info
      // TODO: this solution is not complete but for starters its enough
      auto playerLocation         = camera.getCameraPosition();
      glm::mat4 playerLocationMat = glm::mat4{1.f};
      playerLocationMat = glm::translate(playerLocationMat, playerLocation);
      viewmodelShader.setMat4fv(playerLocationMat, "playerLocation");
      std::vector<glm::mat4> boneTransforms;
      ak.Animate(deltaTime);
      ak.GetBoneTransforms(boneTransforms);
      for (size_t i = 1; i < boneTransforms.size(); i++) {
        viewmodelShader.setBoneTransform(i, boneTransforms[i]);
      }
      viewmodelShader.use();
      ak.Render();
      //----Viewmodel rendering-----

      // TODO: create debug output handler class
      //  handle performance debug output
      {
        if (now - lastTimeTextWasRendered > 1 / textTickRate) {
          lastTimeTextWasRendered = now;
          timeStr                 = "Elapsed time: " +
                                    std::to_string(static_cast<unsigned int>(
                                        std::floor(deltaTime * 1000.f))) +
                                    " ms";
          FPSStr          = "FPS: " + std::to_string(static_cast<unsigned int>(
                                          std::floor(1.f / deltaTime)));
          cameraLocStr    = std::string("CAM location: ") +
                            glm::to_string(camera.getCameraPosition());
          cameraFacingStr = std::string("CAM facing: ") +
                            glm::to_string(camera.getCameraFront());
          entityCountStr =
              std::string("Entities: ") + std::to_string(MAX_ENTITY);
          auto playerPosition = player.getCamera().getCameraPosition();
          playerPositionStr =
              std::string("Player position: ") + glm::to_string(playerPosition);
          auto& playerVelocity =
              componentRegistry.transforms.at(playerEntityID).vel;
          playerVelocityStr =
              std::string("Player velocity: ") + glm::to_string(playerVelocity);
        }
        //  ------ ADDING TEXT RENDER HERE ----------
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
        textrenderer.renderText(playerPositionStr, 0.0f, SCR_HEIGHT - 6 * 24,
                                0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
        textrenderer.renderText(playerVelocityStr, 0.0f, SCR_HEIGHT - 7 * 24,
                                0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
        // ------ ADDING TEXT RENDER HERE ----------
      }
      // ---- Poll shader file changes ----
      std::for_each(observers.begin(), observers.end(),
                    [](auto& observer) { observer->update(); });
      // after reloading these uniforms need to be reset
      shader.set1i(0, "diffuseTexture");
      shader.set1i(1, "shadowMap");
      // ---- Poll shader file changes ----

      // buffer swap(double buffering)
      window.swapBuffers();
      glfwPollEvents(); // special events
    }
  }
  glfwTerminate(); // terminates glfw library, cleanup

  return 0;
}
