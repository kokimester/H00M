#include "Texture.h"

Texture::Texture()
    : textureID(0), height(0), width(0), bitDepth(0), fileLocation(nullptr) {}

Texture::Texture(const char *fileLoc)
    : textureID(0), height(0), width(0), bitDepth(0), fileLocation(nullptr) {
  int length = strlen(fileLoc) + 1;
  if (length == 1) {
    throw std::invalid_argument(
        "Texture constructor received empty file location.");
  }
  fileLocation = new char[length];
  for (int i = 0; i < length; i++) {
    fileLocation[i] = fileLoc[i];
  }
}

Texture::~Texture() { clearTexture(); }

bool Texture::loadTextureAlpha() {
  unsigned char *texData =
      stbi_load(fileLocation, &width, &height, &bitDepth, 0);
  if (!texData) {
    printf("Failed to find texture at location: %s", fileLocation);
    return false;
  }
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  // GL_REPEAT GL_MIRRORED_REPEAT GL_CLAMP GL_CLAMP_BORDER
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  // GL_LINEAR !! GL_NEAREST
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  //.png supports alpha channel (transparency)
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, texData);
  // GL_RGB vagz GL_RGBA texture fuggo !! mindig 0 !!
  glGenerateMipmap(GL_TEXTURE_2D); // egyszeru, de hatekonyabb ha en hozom letre
                                   // a mipmapelt textureket

  glBindTexture(GL_TEXTURE_2D, 0);

  stbi_image_free(texData);
  return true;
}

bool Texture::loadTexture() {
  unsigned char *texData =
      stbi_load(fileLocation, &width, &height, &bitDepth, 0);
  if (!texData) {
    printf("Failed to find texture at location: %s", fileLocation);
    return false;
  }
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  glTexParameteri(
      GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
      GL_REPEAT); // GL_REPEAT GL_MIRRORED_REPEAT GL_CLAMP GL_CLAMP_BORDER
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR); // GL_LINEAR !! GL_NEAREST
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  //.png supports alpha channel (transparency)
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE,
               texData); // GL_RGB vagz GL_RGBA texture fuggo !! mindig 0 !!
  glGenerateMipmap(GL_TEXTURE_2D); // egyszeru, de hatekonyabb ha en hozom letre
                                   // a mipmapelt textureket

  glBindTexture(GL_TEXTURE_2D, 0);

  stbi_image_free(texData);
  return true;
}

void Texture::useTexture() {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureID);
}

void Texture::clearTexture() {
  glDeleteTextures(1, &textureID);
  textureID = 0;
  width = 0;
  height = 0;
  bitDepth = 0;
  delete fileLocation;
}
