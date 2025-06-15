#include "Window.h"

void Window::createCallbacks() {
  glfwSetKeyCallback(mainWindow, handleKeys);
  glfwSetCursorPosCallback(mainWindow, handleMouse);
}

void Window::handleKeys(GLFWwindow *window, int key, [[maybe_unused]] int code,
                        int action, [[maybe_unused]] int mode) {
  Window *theWindow = static_cast<Window *>(glfwGetWindowUserPointer(window));

  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
  if (key >= 0 && key < 1024) {
    if (action == GLFW_PRESS) {
      theWindow->keys[key] = true;
      /* printf("Pressed: %d\n", key); */
    } else if (action == GLFW_RELEASE) {
      theWindow->keys[key] = false;
      /* printf("Released: %d\n", key); */
    }
  }
}

void Window::handleMouse(GLFWwindow *window, double xPos, double yPos) {
  Window *theWindow = static_cast<Window *>(glfwGetWindowUserPointer(window));

  if (theWindow->mouseFirstMoved) {
    theWindow->lastX = xPos;
    theWindow->lastY = yPos;
    theWindow->mouseFirstMoved = false;
  }

  theWindow->xChange = xPos - theWindow->lastX;
  theWindow->yChange = theWindow->lastY - yPos;

  theWindow->lastX = xPos;
  theWindow->lastY = yPos;
}

Window::Window()
    : mainWindow(NULL), fullScreen(false), width(800), height(600),
      bufferWidth(0), bufferHeight(0), mouseFirstMoved(true) {
  for (size_t i = 0; i < 1024; i++) {
    keys[i] = false;
  }
}

Window::Window(int wWidth, int wHeight, bool pFullScreen)
    : mainWindow(NULL), fullScreen(pFullScreen), width(wWidth), height(wHeight),
      bufferWidth(0), bufferHeight(0), mouseFirstMoved(true) {
  for (size_t i = 0; i < 1024; i++) {
    keys[i] = false;
  }
}
int Window::Initialise() {
  if (!glfwInit()) {
    printf("Error initialising GLFW");
    glfwTerminate();
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

  glfwWindowHint(GLFW_FLOATING, GL_TRUE);

  GLFWmonitor *monitor = NULL;
  if (fullScreen) {
    monitor = glfwGetPrimaryMonitor();
  }

  // const GLFWvidmode* mode = glfwGetVideoMode(monitor);
  //
  // glfwWindowHint(GLFW_RED_BITS, mode->redBits);
  // glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
  // glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
  // glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

  mainWindow = glfwCreateWindow(width, height, "H00M", monitor, NULL);
  if (!mainWindow) {
    printf("Error creating GLFW window!");
    glfwTerminate();
    return 1;
  }
  // get buffersize info
  glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

  glfwMakeContextCurrent(mainWindow);

  // Disable VSYNC for maximum FPS
  /* glfwSwapInterval(0); */

  // handle key and mouse input
  createCallbacks();

  // lockoljuk az egeret kozepre
  glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  if (!gladLoadGL()) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }
  // modern extension acces
  /* glewExperimental = GL_TRUE; */

  /* GLenum error = glewInit(); */
  /* if (error != GLEW_OK) { */
  /*   printf("Error: %s", glewGetErrorString(error)); */
  /*   glfwDestroyWindow(mainWindow); */
  /*   glfwTerminate(); */
  /*   return 1; */
  /* } */

  /* glEnable(GL_DEPTH_TEST); */

  std::cout << "Setting viewport to: " << bufferWidth << "x" << bufferHeight
            << std::endl;
  glViewport(0, 0, bufferWidth, bufferHeight);

  glfwSetWindowUserPointer(mainWindow, this);

  // Print various OpenGL information to stdout
  printf("%s: %s\n", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
  printf("GLFW\t %s\n", glfwGetVersionString());
  printf("OpenGL\t %s\n", glGetString(GL_VERSION));
  printf("GLSL\t %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
  return 0;
}

float Window::getXChange() {
  float theChange = xChange;
  xChange = 0.0f;
  return theChange;
}

float Window::getYChange() {
  float theChange = yChange;
  yChange = 0.0f;
  return theChange;
}

void Window::processInput([[maybe_unused]] Camera &camera) {

  // if (glfwGetKey(mainWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
  //	glfwSetWindowShouldClose(mainWindow, true);
  // if (glfwGetKey(mainWindow, GLFW_KEY_1) == GLFW_PRESS)
  //	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  // if (glfwGetKey(mainWindow, GLFW_KEY_2) == GLFW_PRESS)
  //	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

Window::~Window() {
  glfwDestroyWindow(mainWindow);
  glfwTerminate();
}
