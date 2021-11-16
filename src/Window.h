#pragma once
#include <iostream>
#include <stdio.h>

#include <glew/glew.h>
#include <GLFW\glfw3.h>
#include "Camera.h"

class Window
{
private:
	GLFWwindow* mainWindow;

	GLboolean fullScreen;

	GLint width, height;
	GLint bufferWidth, bufferHeight;

	bool keys[1024];

	GLfloat lastX;
	GLfloat lastY;
	GLfloat xChange;
	GLfloat yChange;
	bool mouseFirstMoved; //legelso movement, ha belepunk az alkalmazasba ne ugrojon 180at a kamera

	void createCallbacks();
	static void handleKeys(GLFWwindow* window, int key, int code, int action, int mode);
	static void handleMouse(GLFWwindow* window, double xPos, double yPos);
public:
	Window();

	Window(GLint windowWidht, GLint windowHeight, GLboolean pFullScreen);

	GLFWwindow* getWindow() const { return mainWindow; }

	int Initialise();

	GLint getBufferWidth() { return bufferWidth; }
	GLint getBufferHeight() { return bufferHeight; }

	bool getShouldClose() { return glfwWindowShouldClose(mainWindow); }

	//nem tul oop, dobalgatni kene az ablakot ehelyett
	bool* getKeys() { return keys; }
	
	GLfloat getXChange();
	GLfloat getYChange();


	void swapBuffers() { glfwSwapBuffers(mainWindow); }

	//
	void processInput(Camera& camera);
	//
	~Window();

};

