#define STB_IMAGE_IMPLEMENTATION
#define STB_EASY_FONT_IMPLEMENTATION

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

// ------ ADDING TEXT HERE ----------
/* #include <stb_easy_font.h> */

// ------ ADDING TEXT HERE ----------

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
  for (unsigned int i = 0; i < indiceCount; i += 3) {
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

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
  unsigned int TextureID; // ID handle of the glyph texture
  glm::ivec2 Size;        // Size of glyph
  glm::ivec2 Bearing;     // Offset from baseline to left/top of glyph
  unsigned int Advance;   // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;
unsigned int textVAO, textVBO;

// render line of text
// -------------------
void RenderText(Shader &shader, std::string text, float x, float y, float scale,
                glm::vec3 color, glm::mat4 projection) {
  // activate corresponding render state
  shader.use();
  shader.set1i(0, "text");
  shader.setVec3f(color, "textColor");
  shader.setMat4fv(projection, "projection");
  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(textVAO);

  // iterate through all characters
  std::string::const_iterator c;
  for (c = text.begin(); c != text.end(); c++) {
    Character ch = Characters[*c];

    float xpos = x + ch.Bearing.x * scale;
    float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

    float w = ch.Size.x * scale;
    float h = ch.Size.y * scale;
    // update VBO for each character

    float vertices[6][4] = {
        {xpos, ypos + h, 0.0f, 0.0f},    {xpos, ypos, 0.0f, 1.0f},
        {xpos + w, ypos, 1.0f, 1.0f},

        {xpos, ypos + h, 0.0f, 0.0f},    {xpos + w, ypos, 1.0f, 1.0f},
        {xpos + w, ypos + h, 1.0f, 0.0f}};

    /* for (int k = 0; k < 6; ++k) { */
    /*   for (int l = 0; l < 4; ++l) { */
    /*     std::cout << vertices[k][l] << " "; */
    /*   } */
    /*   std::cout << "\n"; */
    /* } */

    /* float vertices[6][4] = { */
    /*     {10.0f, 10.0f, 0.0f, 0.0f}, {10.0f, 50.0f, 0.0f, 1.0f}, */
    /*     {50.0f, 50.0f, 1.0f, 1.0f}, {10.0f, 10.0f, 0.0f, 0.0f}, */
    /*     {50.0f, 50.0f, 1.0f, 1.0f}, {50.0f, 10.0f, 1.0f, 0.0f}}; */
    // render glyph texture over quad
    /* glBindTexture(GL_TEXTURE_2D, ch.TextureID); */
    std::cout << "Drawing character: " << *c << std::endl;
    std::cout << "Binding textureID: " << ch.TextureID << std::endl;
    std::cout << "Texture coordinates: " << xpos << " " << ypos << std::endl;
    // update content of VBO memory
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    /* glBufferSubData( */
    /*     GL_ARRAY_BUFFER, 0, sizeof(vertices), */
    /*     vertices); // be sure to use glBufferSubData and not glBufferData */
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // render quad
    glDrawArrays(GL_TRIANGLES, 0, 6);
    // now advance cursors for next glyph (note that advance is number of 1/64
    // pixels)
    x += (ch.Advance >> 6) *
         scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount
                // of 1/64th pixels by 64 to get amount of pixels))
  }
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

int main() {
  {
    Window window = Window(SCR_WIDTH, SCR_HEIGHT, GLFW_FALSE);
    window.Initialise();

    // create triangle
    createTriangle();

    // OpenGL state
    // ------------
    /* glEnable(GL_CULL_FACE); */
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#if 0
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

      pointLights[0] = PointLight(0.0f, 0.0f, 1.0f, 0.0f, 0.3f, 0.0f, 1.0f,
                                  0.0f, 0.3f, 0.2f, 0.1f);
      /* pointLightCount++; */

      pointLights[1] = PointLight(0.0f, 1.0f, 0.0f, 0.0f, 0.3f, -4.0f, 2.0f,
                                  0.0f, 0.3f, 0.1f, 0.1f);

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
      Texture brickTexture("../textures/brick.png");
      brickTexture.loadTextureAlpha();
      Texture dirtTexture("../textures/dirt.png");
      dirtTexture.loadTextureAlpha();
      Texture plainTexture("../textures/floor.png");
      plainTexture.loadTextureAlpha();
#endif
    // compile shaders
    Shader shader("../shaders/vertex_source.glsl.vert",
                  "../shaders/fragment_source.glsl.frag");

    glm::mat4 projection(1.f);
    projection =
        glm::perspective(45.0f,
                         (static_cast<GLfloat>(window.getBufferWidth()) /
                          static_cast<GLfloat>(window.getBufferHeight())),
                         0.1f, 100.0f);

    //---TEXT---
    // compile and setup the shader
    // ----------------------------
    Shader textshader("../shaders/text.vert", "../shaders/text.frag");
    glm::mat4 textprojection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH),
                                          0.0f, static_cast<float>(SCR_HEIGHT));
    textshader.use();
    textshader.set1i(1, "text");
    textshader.setMat4fv(textprojection, "projection");

    // FreeType
    // --------
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft)) {
      std::cout << "ERROR::FREETYPE: Could not init FreeType Library"
                << std::endl;
      return -1;
    }

    // find path to font
    /* std::string font_name =
     * FileSystem::getPath("../fonts/Consolas-Bold.ttf"); */
    std::string font_name = "../fonts/Consolas-Bold.ttf";
    if (font_name.empty()) {
      std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
      return -1;
    }

    // load font as face
    FT_Face face;
    unsigned int textureID;
    if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
      std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
      return -1;
    } else {
      // set size to load glyphs as
      FT_Set_Pixel_Sizes(face, 0, 48);

      // disable byte-alignment restriction
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      // load first 128 characters of ASCII set
      std::cout << "Loading characters into memory\n";
      //-----------------------------------
      // initialize variables to keep track of texture atlas size
      int atlas_width = 0;
      int atlas_height = 0;

      // loop through the characters and load their glyphs
      for (unsigned char c = 0; c < 128; c++) {
        // load glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
          std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
          continue;
        }

        // update atlas size variables
        atlas_width += face->glyph->bitmap.width;
        atlas_height =
            std::max(atlas_height, static_cast<int>(face->glyph->bitmap.rows));
      }

      // generate texture for the atlas
      glGenTextures(1, &textureID);
      glBindTexture(GL_TEXTURE_2D, textureID);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas_width, atlas_height, 0,
                   GL_RED, GL_UNSIGNED_BYTE, nullptr);

      // set texture options
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      // initialize x position for next glyph
      int x = 0;

      // loop through the characters again and add their glyphs to the atlas
      for (unsigned char c = 0; c < 128; c++) {
        // load glyph
        if (FT_Load_Char(face, static_cast<char>(c), FT_LOAD_RENDER)) {
          std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
          continue;
        }
        glTextureSubImage2D(textureID, 0, x, 0, face->glyph->bitmap.width,
                            face->glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE,
                            face->glyph->bitmap.buffer);

        // now store character for later use
        Character character = {
            textureID,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)};
        Characters.insert(std::pair<char, Character>(c, character));
        std::cout << "Saving character " << c << " textureID: " << textureID
                  << std::endl;
        // update x position for next glyph
        x += face->glyph->bitmap.width;
      }

      glBindTexture(GL_TEXTURE_2D, 0);
    }
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    //--------------------------------------------------------------------
#if 0
      for (unsigned char c = 0; c < 128; c++) {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
          std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
          continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        if (face->glyph->bitmap.buffer == nullptr) {
          std::cout << "ERROR::FREETYTPE: Bitmap buffer is nullptr"
                    << std::endl;
          continue;
        }
        if (!(face->glyph->bitmap.width > 0 && face->glyph->bitmap.rows > 0)) {
          std::cout << "ERROR::FREETYTPE: Bitmap size is not correct"
                    << std::endl;
          continue;
        }
        // dummy
        /* unsigned char white_pixel[] = {255}; */
        /* glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, */
        /*              GL_UNSIGNED_BYTE, white_pixel); */

        /* glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, */
        /*              face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
         */
        /*              face->glyph->bitmap.buffer); */
        glTextureSubImage2D(texture, 0, x, 0, face->glyph->bitmap.width,
                            face->glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE,
                            face->glyph->bitmap.buffer);
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)};
        Characters.insert(std::pair<char, Character>(c, character));
        x += face->glyph->bitmap.width;
      }
      glBindTexture(GL_TEXTURE_2D, 0);
    }
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
#endif

    std::cout << "Destroying freetype, done importing\n";
    std::cout << "Imported " << Characters.size()
              << " characters into memory\n";
    for (auto &[c, ch] : Characters) {
      if (ch.TextureID == 0) {
        std::cout << "Character " << c << " has no texture!" << std::endl;
      }
    }

    // configure VAO/VBO for texture quads
    // -----------------------------------
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    /* glEnableVertexAttribArray(1); */
    /* glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), */
    /*                       (const void *)(6 * sizeof(GLfloat))); */

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //---TEXT---
    /*
    char buffer[99999]; // vertices
    const char *text = "Hello, HUD!";
    int x = 300, y = 300;

    // --- Text VBO/VAO ---
    GLuint textVAO, textVBO;
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);

    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                          (void *)0);
    //glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, (void *)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    */

    while (!window.getShouldClose()) // returns true if window is closed
    {
#if 0
        window.processInput(
            camera); // 1 es 2 re vonalakra csereli a haromszogeket

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
#endif

      // render stuff
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      // old clear
      /* glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); */
      // new clear
      glClear(GL_COLOR_BUFFER_BIT);

#if 0

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
        model = glm::rotate(model, glm::radians(-90.0f),
                            glm::vec3(1.0f, 0.0f, 0.0f));
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
        model = glm::rotate(model, glm::radians(-90.0f),
                            glm::vec3(1.0f, 0.0f, 0.0f));
        shader.setMat4fv(model, "model");
        shader.use();
        shader.useMaterial(shinyMaterial, "material.shininess",
                           "material.specularIntensity");
        dancer.renderModel();
        shader.unuse();

#endif
      // ------ ADDING TEXT HERE ----------

      /* glClearColor(0.2f, 0.3f, 0.3f, 1.0f); */
      /* glClear(GL_COLOR_BUFFER_BIT); */
      glDisable(GL_DEPTH_TEST);
      /* RenderText(textshader, "This is sample text", 10.0f, 10.0f, 1.0f, */
      /*            glm::vec3(1.f, 1.f, 1.f)); */
      /* RenderText(textshader, "(C) LearnOpenGL.com", 540.0f, 570.0f, 0.5f,
       */
      /*            glm::vec3(0.3, 0.7f, 0.9f)); */
      RenderText(
          textshader, "ABCDEabcdeABCDEabcdeAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
          25.0f, 25.0f, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f), textprojection);
      glEnable(GL_DEPTH_TEST);

      GLenum err;
      while ((err = glGetError()) != GL_NO_ERROR) {
        std::cout << "OpenGL error: " << err << std::endl;
      }

      /*
      int w, h;
      w = window.getBufferWidth();
      h = window.getBufferHeight();
      int num_quads =
          stb_easy_font_print(x, y, (char *)text, NULL, buffer,
      sizeof(buffer)); std::cout << num_quads << '\n'; glm::mat4 ortho =
      glm::ortho(0.f, (float)w, 0.f, (float)h, -1.f, 1.f);

      textshader.use();
      textshader.setMat4fv(ortho, "uOrtho");
      //glUniformMatrix4fv(glGetUniformLocation(textShader, "uOrtho"), 1,
      //                   GL_FALSE, ortho);
      textshader.setVec3f(glm::fvec3{1.f, 1.f, 1.f}, "uColor");
      // glUniform3f(glGetUniformLocation(textShader,
      "uColor"), 1.0, 1.0, 1.0);

      textshader.use();

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      glBindVertexArray(textVAO);
      glBindBuffer(GL_ARRAY_BUFFER, textVBO);
      glBufferData(GL_ARRAY_BUFFER, num_quads * 4 * 16, buffer,
                   GL_DYNAMIC_DRAW);

      glDisable(GL_DEPTH_TEST);
      glDrawArrays(GL_QUADS, 0, num_quads * 4);
      glEnable(GL_DEPTH_TEST);
      textshader.unuse();
      */
      // ------ ADDING TEXT HERE ----------

      // buffer swap(double buffering)
      window.swapBuffers();
      glfwPollEvents(); // special events
    }
    for (auto itr : meshList) {
      delete itr;
    }
  }
  glfwTerminate(); // terminates glfw library, cleanup

  return 0;
}
