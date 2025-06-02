#pragma once
#include <iostream>
#include <stdio.h>

#include "Camera.h"
/* #include <GLES3/gl3.h> */
#include <glad/glad.h>
/* #include <GL/glew.h> */
#include <GLFW/glfw3.h>

class Window {
private:
  GLFWwindow *mainWindow;

  bool fullScreen;

  int width, height;
  int bufferWidth, bufferHeight;

  bool keys[1024];

  float lastX;
  float lastY;
  float xChange;
  float yChange;
  bool mouseFirstMoved; // legelso movement, ha belepunk az alkalmazasba ne
                        // ugrojon 180at a kamera

  void createCallbacks();
  static void handleKeys(GLFWwindow *window, int key, int code, int action,
                         int mode);
  static void handleMouse(GLFWwindow *window, double xPos, double yPos);

public:
  Window();

  Window(int windowWidht, int windowHeight, bool pFullScreen);

  GLFWwindow *getWindow() const { return mainWindow; }

  int Initialise();

  int getBufferWidth() { return bufferWidth; }
  int getBufferHeight() { return bufferHeight; }

  bool getShouldClose() { return glfwWindowShouldClose(mainWindow); }

  // nem tul oop, dobalgatni kene az ablakot ehelyett
  bool *getKeys() { return keys; }

  float getXChange();
  float getYChange();

  void swapBuffers() { glfwSwapBuffers(mainWindow); }

  //
  void processInput(Camera &camera);
  //
  ~Window();
};
