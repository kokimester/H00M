#include "Text.h"

TextRenderer::TextRenderer(GLuint screenWidth, GLuint screenHeight)
    : width{screenWidth}, height{screenHeight} {}

int TextRenderer::init(const fs::path &vertexShaderPath,
                       const fs::path &fragmentShaderPath,
                       const fs::path &fontPath) {
  if (shader.compile_and_link(vertexShaderPath.string().c_str(),
                              fragmentShaderPath.string().c_str())) {
    std::cout << "Text shader compilation or linking error!\n";
    return -1;
  }
  glm::mat4 textprojection = glm::ortho(0.0f, static_cast<float>(width), 0.0f,
                                        static_cast<float>(height));
  // this has to match the GL_TEXTURE{text} in the rendering function
  shader.set1i(1, "text");
  shader.setMat4fv(textprojection, "projection");
  // FreeType
  // --------
  FT_Library ft;
  // All functions return a value different than 0 whenever an error occurred
  if (FT_Init_FreeType(&ft)) {
    std::cout << "ERROR::FREETYPE: Could not init FreeType Library"
              << std::endl;
    return -1;
  }

  // load font as face
  FT_Face face;
  if (FT_New_Face(ft, fontPath.string().c_str(), 0, &face)) {
    std::cout << "ERROR::FREETYPE: Failed to load font: " << fontPath
              << std::endl;
    return -2;
  }

  // set size to load glyphs as
  FT_Set_Pixel_Sizes(face, 0, 48);

  // disable byte-alignment restriction
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // load first 128 characters of ASCII set
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width,
                 face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
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
  }
  // destroy FreeType once we're finished
  FT_Done_Face(face);
  FT_Done_FreeType(ft);
  // configure VAO/VBO for texture quads
  // -----------------------------------
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  return 0;
}

void TextRenderer::updateScreenParameters(GLuint screenWidth,
                                          GLuint screenHeight) {
  width = screenWidth;
  height = screenHeight;
}

void TextRenderer::renderText(std::string_view text, float x, float y,
                              float scale, glm::vec3 color) {
  // activate corresponding render state
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  shader.set3f(color, "textColor");
  shader.use();
  glActiveTexture(GL_TEXTURE1);
  glBindVertexArray(VAO);

  // iterate through all characters
  std::string_view::const_iterator c;
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
    // render glyph texture over quad
    glBindTexture(GL_TEXTURE_2D, ch.TextureID);
    // update content of VBO memory
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // be sure to use glBufferSubData and not glBufferData
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // render quad
    glDrawArrays(GL_TRIANGLES, 0, 6);
    // now advance cursors for next glyph (note that advance is number of 1/64
    // pixels)
    x += (ch.Advance >> 6) * scale;
    // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount
    // of 1/64th pixels by 64 to get amount of pixels))
  }
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
}
