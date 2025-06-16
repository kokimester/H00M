#define STB_IMAGE_IMPLEMENTATION
/* #include "stb_image.h" */

#include <cstddef>
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
#include <ft2build.h>
#include <glad/glad.h>
#include FT_FREETYPE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "Material.h"
#include "Mesh.h"
#include "Model.h"
#include "PointLight.h"
#include "Shader.h"
#include "Spotlight.h"
#include "Texture.h"
#include "Window.h"

#include <assimp/Importer.hpp>

// TODO: Globals, move somewhere else, as static perhaps
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
GLfloat lastTimeTextWasRendered = 0.0f;
std::string timeStr, FPSStr;

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
    /* glm::vec3 v1(vertices[in1] - vertices[in0], */
    /*              vertices[in1 + 1] - vertices[in0 + 1], */
    /*              vertices[in1 + 2] - vertices[in0 + 2]); */
    /* glm::vec3 v2(vertices[in2] - vertices[in0], */
    /*              vertices[in2 + 1] - vertices[in0 + 1], */
    /*              vertices[in2 + 2] - vertices[in0 + 2]); */
    glm::vec3 v0(vertices[in0], vertices[in0 + 1], vertices[in0 + 2]);
    glm::vec3 v1(vertices[in1], vertices[in1 + 1], vertices[in1 + 2]);
    glm::vec3 v2(vertices[in2], vertices[in2 + 1], vertices[in2 + 2]);

    /* glm::vec3 normal = glm::cross(v1, v2); */
    glm::vec3 normal = v0 + v1 + v2;
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

    /* in0 += normalOffset; */
    /* in1 += normalOffset; */
    /* in2 += normalOffset; */

    /* vertices[in0] += normal.x; */
    /* vertices[in0 + 1] += normal.y; */
    /* vertices[in0 + 2] += normal.z; */
    /* vertices[in1] += normal.x; */
    /* vertices[in1 + 1] += normal.y; */
    /* vertices[in1 + 2] += normal.z; */
    /* vertices[in2] += normal.x; */
    /* vertices[in2 + 1] += normal.y; */
    /* vertices[in2 + 2] += normal.z; */
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
  unsigned int indices[] = {0, 3, 1, 1, 3, 2, 2, 3, 0, 0, 1, 2};

  GLfloat vertices[] = {
      // x     y     z		  u     v  normals:  nx   ny   nz
      -1.0f, -1.0f, -0.6f, 0.0f, 0.0f, 0.0f, 0.0f,  0.0f,  0.0f, -1.0f, 1.0f,
      0.5f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f, -1.0f, -0.6f, 1.0f, 0.0f,  0.0f,
      0.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.5f, 1.0f,  0.0f,  0.0f, 0.0f};

  // uj objektum

  unsigned int floorIndices[]{0, 1, 2, 1, 2, 3};

  GLfloat floorVertices[]{-10.0f, 0.0f, -10.f, 0.0f,  0.0f,  0.0f, -1.0f, 0.0f,
                          10.0f,  0.0f, -10.f, 10.0f, 0.0f,  0.0f, -1.0f, 0.0f,
                          -10.0f, 0.0f, 10.f,  0.0f,  10.0f, 0.0f, -1.0f, 0.0f,
                          10.0f,  0.0f, 10.f,  10.0f, 10.0f, 0.0f, -1.0f, 0.0f};

  calcAverageNormals(indices, 12, vertices, 32, 8, 5);

  meshList.push_back(Mesh());
  meshList.push_back(Mesh());
  meshList.push_back(Mesh());

  meshList[0].createMesh(vertices, indices, sizeof(vertices) / sizeof(GLfloat),
                         sizeof(indices) / sizeof(GLfloat));
  meshList[1].createMesh(vertices, indices, sizeof(vertices) / sizeof(GLfloat),
                         sizeof(indices) / sizeof(GLfloat));
  meshList[2].createMesh(floorVertices, floorIndices,
                         sizeof(floorVertices) / sizeof(GLfloat),
                         sizeof(floorIndices) / sizeof(GLfloat));

  /* meshList[0].createMesh(vertices, indices, 32, 12); */
  /* meshList[1].createMesh(vertices, indices, 32, 12); */
  /* meshList[2].createMesh(floorVertices, floorIndices, 32, 6); */
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
        DirectionalLight(1.0f, 1.0f, 1.0f, 0.1f, 0.2f, 0.0f, -1.0f, 0.0f);

    PointLight pointLights[MAX_POINT_LIGHTS];

    Spotlight spotLights[MAX_SPOT_LIGHTS];

    unsigned int pointLightCount = 0;
    unsigned int spotLightCount = 0;

    pointLights[0] = PointLight(0.0f, 0.0f, 1.0f, 0.0f, 0.3f, 0.0f, 1.0f, 0.0f,
                                0.3f, 0.2f, 0.1f);
    /* pointLightCount++; */

    pointLights[1] = PointLight(0.0f, 1.0f, 0.0f, 0.0f, 0.3f, -4.0f, 2.0f, 0.0f,
                                0.3f, 0.1f, 0.1f);

    /* pointLightCount++; */

    // angles in degrees
    float inner = 5.f;
    float outer = 20.f;

    spotLights[0] = Spotlight(
        1.0f, 1.0f, 1.0f, 0.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 20.0f, cos(glm::radians(inner)), cos(glm::radians(outer)));

    spotLightCount++;

    spotLights[1] =
        Spotlight(1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 5.0f, 0.0f, 0.0f, -100.0f,
                  -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 20.0f, inner, outer);

    /* spotLightCount++; */

    // material
    Material shinyMaterial = Material(4.0f, 256);
    Material dullMaterial = Material(0.3f, 2);

    Model ah60;
    Model dancer;
    try {
      /* ah60.loadModel("Models/3ds file.3DS"); */
      /* dancer.loadModel("Models/Bodymesh.obj"); */
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
    if (shader.compile_and_link("../shaders/vertex_source.glsl.vert",
                                "../shaders/fragment_source.glsl.frag")) {
      std::cout << "Shader compilation or linking error!\n";
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


    //------------------TEXT------------------

    while (!window.getShouldClose()) // returns true if window is closed
    {
      glEnable(GL_DEPTH_TEST);
      // 1 es 2 re vonalakra csereli a haromszogeket
      window.processInput(camera);

      // returns elapsed time in seconds as a double
      GLfloat now = glfwGetTime();
      deltaTime = now - lastTime;
      lastTime = now;

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

      glm::mat4 view = camera.calculateViewMatrix();
      // berakjuk a kamera helyet igy igazodik a visszaverodes
      shader.setVec3f(camera.getCameraPosition(), "eyePosition");
      shader.setMat4fv(view, "view");
      shader.setMat4fv(projection, "projection");

      glm::mat4 model(1.f);

      model = glm::translate(model, glm::vec3(0.0f, -1.0f, -3.0f));
      // model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
      model = glm::rotate(model, glm::radians(triRotation),
                          glm::vec3(0.0f, 1.0f, 0.0f));
      triRotation += 10.f * deltaTime;
      shader.setMat4fv(model, "model");

      // hasznaljuk a fenyt
      //==================================

      shader.setDirectionalLight(mainLight);
      shader.setPointLights(pointLights, pointLightCount);
      shader.setSpotLights(spotLights, spotLightCount);

      //==================================

      // rajzolo függvény, használjuk a shaderprogramot:
      shader.use();

      // beallitjuk a texturet
      brickTexture.useTexture();
      shader.useMaterial(shinyMaterial, "material.shininess",
                         "material.specularIntensity");

      // letrehozzuk a trianglet
      meshList[0].renderMesh();

      shader.unuse();

      // magasabbra helyezunk megegy tetraedert
      model = glm::mat4(1.f);
      model = glm::translate(model, glm::vec3(0.0f, 1.0f, -3.0f));
      model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
      shader.setMat4fv(model, "model");

      shader.use();

      // beallitjuk a texturet
      dirtTexture.useTexture();
      shader.useMaterial(dullMaterial, "material.shininess",
                         "material.specularIntensity");

      // letrehozzuk a trianglet
      meshList[1].renderMesh();

      // end of render stuff, bufferswap, and event check
      shader.unuse();

      // bealltijuk a foldet

      model = glm::mat4(1.f);
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

      // ah60
      model = glm::mat4(1.f);
      model = glm::translate(model, glm::vec3(3.0f, -2.0f, 0.0f));
      model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
      model =
          glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
      shader.setMat4fv(model, "model");
      shader.use();
      shader.useMaterial(shinyMaterial, "material.shininess",
                         "material.specularIntensity");
      ah60.renderModel();
      shader.unuse();

      // dancer
      model = glm::mat4(1.f);
      model = glm::translate(model, glm::vec3(-3.0f, 2.0f, 0.0f));
      model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
      model =
          glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
      shader.setMat4fv(model, "model");
      shader.use();
      shader.useMaterial(shinyMaterial, "material.shininess",
                         "material.specularIntensity");
      dancer.renderModel();
      shader.unuse();

      if (now - lastTimeTextWasRendered > 0.1f) {
        lastTimeTextWasRendered = now;
        timeStr = "Elapsed time: " +
                  std::to_string(static_cast<unsigned int>(
                      std::floor(deltaTime * 1000.f))) +
                  " ms";
        FPSStr = "FPS: " + std::to_string(static_cast<unsigned int>(
                               std::floor(1.f / deltaTime)));
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
