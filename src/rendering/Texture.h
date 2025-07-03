#pragma once

#include <stb_image.h>

#include <exception>
#include <glad/glad.h>
#include <stdexcept>
#include <string.h>

//=====================================
//			EXCEPTIONS
//=====================================
// std::invalid_argument in constructor
//
//
class Texture {
private:
  unsigned int textureID;
  int width, height, bitDepth;
  char *fileLocation;

public:
  Texture();
  Texture(const char *fileLoc);
  ~Texture();

  unsigned int getID() const {return textureID;}

  bool loadTexture();
  bool loadTextureAlpha();

  void useTexture();
  void clearTexture();
};
