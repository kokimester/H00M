#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <string>

#include <cmath>
#include <exception>
#include <math.h>
#include <stdexcept>
#include <stdlib.h>
#include <vector>

#include <GLFW/glfw3.h>
#include <glad/glad.h>

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

// GLuint VAO, VBO, IBO; shader, uniformModel, uniformProjection

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

std::vector<Mesh *> meshList;

bool direction = true;
float triOffset = 0.0f;
float triMaxOffset = 0.8f;
float triIncrement = 0.0005f;
float triRotIncrement = 0.02f;
float triRotation = 0.0f;

void calcAverageNormals(unsigned int *indices, unsigned int indiceCount,
                        GLfloat *vertices, unsigned int verticeCount,
                        unsigned int vLength, unsigned int normalOffset) {
  for (int i = 0; i < indiceCount; i += 3) {
    unsigned int in0 = indices[i] * vLength;
    unsigned int in1 = indices[i + 1] * vLength;
    unsigned int in2 = indices[i + 2] * vLength;
    glm::vec3 v1(vertices[in1] - vertices[in0],
                 vertices[in1 + 1] - vertices[in0 + 1],
                 vertices[in1 + 2] - vertices[in0 + 2]);
    glm::vec3 v2(vertices[in2] - vertices[in0],
                 vertices[in2 + 1] - vertices[in0 + 1],
                 vertices[in2 + 2] - vertices[in0 + 2]);

    glm::vec3 normal = glm::cross(v1, v2);
    normal = glm::normalize(normal);

    in0 += normalOffset;
    in1 += normalOffset;
    in2 += normalOffset;

    vertices[in0] += normal.x;
    vertices[in0 + 1] += normal.y;
    vertices[in0 + 2] += normal.z;
    vertices[in1] += normal.x;
    vertices[in1 + 1] += normal.y;
    vertices[in1 + 2] += normal.z;
    vertices[in2] += normal.x;
    vertices[in2 + 1] += normal.y;
    vertices[in2 + 2] += normal.z;
  }
  for (int i = 0; i < verticeCount / vLength; i++) {
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

  Mesh *obj1 = new Mesh();
  obj1->createMesh(vertices, indices, 32, 12);
  meshList.push_back(obj1);

  Mesh *obj2 = new Mesh();
  obj2->createMesh(vertices, indices, 32, 12);
  meshList.push_back(obj2);

  Mesh *obj3 = new Mesh();
  obj3->createMesh(floorVertices, floorIndices, 32, 6);
  meshList.push_back(obj3);
}

int main() {
  {
    Window window = Window(1366, 768, GLFW_FALSE);
    window.Initialise();

    // create triangle
    createTriangle();

    // create camera
    Camera camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
                  -90.0f, 0.0f, 2.0f, 0.15f);

    // light source
    DirectionalLight mainLight =
        DirectionalLight(1.0f, 1.0f, 1.0f, 0.0f, 0.2f, 0.0f, -1.0f, 0.0f);

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

    spotLights[0] = Spotlight(1.0f, 1.0f, 1.0f, 0.0f, 2.0f, 0.0f, 0.0f, 0.0f,
                              0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 20.0f);

    /* spotLightCount++; */

    spotLights[1] = Spotlight(1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 5.0f, 0.0f, 0.0f,
                              -100.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 20.0f);

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
    Texture brickTexture("textures/brick.png");
    brickTexture.loadTextureAlpha();
    Texture dirtTexture("textures/dirt.png");
    dirtTexture.loadTextureAlpha();
    Texture plainTexture("textures/floor.png");
    plainTexture.loadTextureAlpha();

    // compile shaders
    Shader shader("../shaders/vertex_source.glsl.vert",
                  "../shaders/fragment_source.glsl.frag");

    glm::mat4 projection(1.f);
    projection = glm::perspective(
        45.0f, (GLfloat)(window.getBufferWidth() / window.getBufferHeight()),
        0.1f, 100.0f);

    while (!window.getShouldClose()) // returns true if window is closed
    {
      window.processInput(
          camera); // 1 es 2 re vonalakra csereli a haromszogeket

      GLfloat now = glfwGetTime();
      deltaTime = now - lastTime;
      lastTime = now;

      glfwPollEvents(); // special events

      camera.keyControl(window.getKeys(), deltaTime);
      camera.mouseControl(window.getXChange(), window.getYChange());

      if (camera.isFlashlightOn()) {
        spotLights[0].update(camera.getCameraPosition(),
                             camera.getCameraFront(), camera.getRight());
      } else {
        spotLights[0].disable();
      }

      // render stuff
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glm::mat4 view = camera.calculateViewMatrix();
      // berakjuk a kamera helyet igy igazodik a visszaverodes
      shader.setVec3f(camera.getCameraPosition(), "eyePosition");

      shader.setMat4fv(view, "view");
      shader.setMat4fv(projection, "projection");

      glm::mat4 model(1.f);

      model = glm::translate(model, glm::vec3(0.0f, -1.0f, -3.0f));
      // model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
      // model = glm::rotate(model, glm::radians(triRotation),
      // glm::vec3(0.0f, 1.0f, 0.0f));
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
      meshList[0]->renderMesh();

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
      meshList[1]->renderMesh();

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
      meshList[2]->renderMesh();
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

      // buffer swap(double buffering)
      window.swapBuffers();
    }
    std::cout << "GLFW terminate" << std::endl;

    for (auto itr : meshList) {
      delete itr;
    }
  }
  glfwTerminate(); // terminates glfw library, cleanup

  return 0;
}
