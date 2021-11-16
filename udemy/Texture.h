#pragma once

#include <glew/glew.h>
#include "stb_image.h"

#include <string.h>
#include <exception>
#include <stdexcept>

//=====================================
//			EXCEPTIONS
//=====================================
//std::invalid_argument in constructor
//
//
class Texture
{
private:
	GLuint textureID;
	int width, height, bitDepth;
	char* fileLocation;

public:
	Texture();
	Texture(const char* fileLoc);
	~Texture();

	bool loadTexture();
	bool loadTextureAlpha();

	void useTexture();
	void clearTexture();
};

