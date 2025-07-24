#include "Utility.h"
#include <print>
#include <glad/glad.h>
#include <string_view>

void handleGLerrors(){
auto GLerrorToString = [](GLenum error) -> std::string_view {
        {
          switch (error)
          {
            // opengl 2 errors (8)
            case GL_NO_ERROR:
              return "GL_NO_ERROR";

            case GL_INVALID_ENUM:
              return "GL_INVALID_ENUM";

            case GL_INVALID_VALUE:
              return "GL_INVALID_VALUE";

            case GL_INVALID_OPERATION:
              return "GL_INVALID_OPERATION";

            case GL_STACK_OVERFLOW:
              return "GL_STACK_OVERFLOW";

            case GL_STACK_UNDERFLOW:
              return "GL_STACK_UNDERFLOW";

            case GL_OUT_OF_MEMORY:
              return "GL_OUT_OF_MEMORY";

            case GL_TABLE_TOO_LARGE:
              return "GL_TABLE_TOO_LARGE";

            // opengl 3 errors (1)
            case GL_INVALID_FRAMEBUFFER_OPERATION:
              return "GL_INVALID_FRAMEBUFFER_OPERATION";

            // gles 2, 3 and gl 4 error are handled by the switch above
            default:
              return "Unhandled error case in error to string evaluation";
          }
        }
      };
      // Print GL errors
      GLenum err;
      while ((err = glGetError()) != GL_NO_ERROR)
      {
        std::println("OpenGL error: {} ({})", GLerrorToString(err),err);
      }
}

auto convertAssimpMatToGLM(const aiMatrix4x4& AssimpMatrix) -> glm::mat4 {
  glm::mat4 m{1.f};
  m[0][0] = AssimpMatrix.a1; m[0][1] = AssimpMatrix.a2; m[0][2] = AssimpMatrix.a3; m[0][3] = AssimpMatrix.a4;
  m[1][0] = AssimpMatrix.b1; m[1][1] = AssimpMatrix.b2; m[1][2] = AssimpMatrix.b3; m[1][3] = AssimpMatrix.b4;
  m[2][0] = AssimpMatrix.c1; m[2][1] = AssimpMatrix.c2; m[2][2] = AssimpMatrix.c3; m[2][3] = AssimpMatrix.c4;
  m[3][0] = AssimpMatrix.d1; m[3][1] = AssimpMatrix.d2; m[3][2] = AssimpMatrix.d3; m[3][3] = AssimpMatrix.d4;
  return m;
}

auto convertAssimpMatToGLM(const aiMatrix3x3& AssimpMatrix) -> glm::mat4 {
  glm::mat4 m{1.f};
  m[0][0] = AssimpMatrix.a1; m[0][1] = AssimpMatrix.a2; m[0][2] = AssimpMatrix.a3; m[0][3] = 0;
  m[1][0] = AssimpMatrix.b1; m[1][1] = AssimpMatrix.b2; m[1][2] = AssimpMatrix.b3; m[1][3] = 0;
  m[2][0] = AssimpMatrix.c1; m[2][1] = AssimpMatrix.c2; m[2][2] = AssimpMatrix.c3; m[2][3] = 0;
  m[3][0] = 0;               m[3][1] = 0;               m[3][2] = 0;               m[3][3] = 1;
  return m;
};