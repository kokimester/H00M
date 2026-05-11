#include "Utility.h"
#include <array>
#include <glad/glad.h>
#include <print>
#include <ranges>
#include <string_view>

#ifdef __linux__
#include "FileObserverLinux.h"
#elif _WIN32
#include "FileObserverWindows.h"
#else
#error "Unsupported OS"
#endif

void handleGLerrors() {
  auto GLerrorToString = [](GLenum error) -> std::string_view {
    {
      switch (error) {
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
  while ((err = glGetError()) != GL_NO_ERROR) {
    std::println("OpenGL error: {} ({})", GLerrorToString(err), err);
  }
}

std::unique_ptr<FileObserver> fileObserverFactory() {
#ifdef __linux__
  return std::make_unique<FileObserverLinux>();
#elif _WIN32
  return std::make_unique<FileObserverWindows>();
#else
#error "Unsupported OS"
#endif
}

auto convertAssimpMatToGLM(const aiMatrix4x4& AssimpMatrix) -> glm::mat4 {
  glm::mat4 m{1.f};
  m[0][0] = AssimpMatrix.a1;
  m[0][1] = AssimpMatrix.a2;
  m[0][2] = AssimpMatrix.a3;
  m[0][3] = AssimpMatrix.a4;
  m[1][0] = AssimpMatrix.b1;
  m[1][1] = AssimpMatrix.b2;
  m[1][2] = AssimpMatrix.b3;
  m[1][3] = AssimpMatrix.b4;
  m[2][0] = AssimpMatrix.c1;
  m[2][1] = AssimpMatrix.c2;
  m[2][2] = AssimpMatrix.c3;
  m[2][3] = AssimpMatrix.c4;
  m[3][0] = AssimpMatrix.d1;
  m[3][1] = AssimpMatrix.d2;
  m[3][2] = AssimpMatrix.d3;
  m[3][3] = AssimpMatrix.d4;
  return m;
}

auto convertAssimpMatToGLM(const aiMatrix3x3& AssimpMatrix) -> glm::mat4 {
  glm::mat4 m{1.f};
  m[0][0] = AssimpMatrix.a1;
  m[0][1] = AssimpMatrix.a2;
  m[0][2] = AssimpMatrix.a3;
  m[0][3] = 0;
  m[1][0] = AssimpMatrix.b1;
  m[1][1] = AssimpMatrix.b2;
  m[1][2] = AssimpMatrix.b3;
  m[1][3] = 0;
  m[2][0] = AssimpMatrix.c1;
  m[2][1] = AssimpMatrix.c2;
  m[2][2] = AssimpMatrix.c3;
  m[2][3] = 0;
  m[3][0] = 0;
  m[3][1] = 0;
  m[3][2] = 0;
  m[3][3] = 1;
  return m;
};

int validateShaderFiles(const std::filesystem::path& projectPath,
                        const std::filesystem::path& shaderDir,
                        const std::filesystem::path& vertexShaderFile,
                        const std::filesystem::path& fragmentShaderFile,
                        Shader& shader) {
  auto vertexPath   = projectPath / shaderDir / vertexShaderFile;
  auto fragmentPath = projectPath / shaderDir / fragmentShaderFile;
  if (!std::filesystem::exists(vertexPath) ||
      !std::filesystem::exists(fragmentPath)) {
    std::cout << "ERROR::SHADER::MODEL Failed to load shader files."
              << std::endl;
    std::cout << vertexPath.string() << std::endl;
    std::cout << fragmentPath.string() << std::endl;
    return -1;
  }
  std::string vertexFileName   = vertexPath.string();
  std::string fragmentFileName = fragmentPath.string();
  if (shader.compile_and_link(vertexFileName.c_str(),
                              fragmentFileName.c_str())) {
    std::cout << "Shader compilation or linking error!\n";
    return -2;
  }
  return 0;
}

int loadshader(std::filesystem::path projectPath,
               const std::string& shaderDirStr, std::string_view shaderName,
               Shader& shader) {
  std::string_view vertexFileExt = ".vert";
  std::string_view fragFileExt   = ".frag";
  std::string vFilePath = std::string{shaderName} + std::string{vertexFileExt};
  std::string fFilePath = std::string{shaderName} + std::string{fragFileExt};
  auto shaderDir        = std::filesystem::path(shaderDirStr);
  auto vFile            = std::filesystem::path(vFilePath);
  auto fFile            = std::filesystem::path(fFilePath);
  if (validateShaderFiles(projectPath, shaderDir, vFile, fFile, shader)) {
    std::cout << "Error occured while validating shader files!\n";
    return -1;
  }
  std::cout << "Succesfully built shader: " << shaderName << "\n";
  return 0;
};

bool isValidProjectPath(std::filesystem::path& projectPath) {
  // TODO: move this elsewhere
  std::string_view projectName = "H00M";
  while (projectPath != projectPath.root_path() &&
         projectPath.filename() != projectName) {
    projectPath = projectPath.parent_path();
  }
  if (projectPath == projectPath.root_path()) {
    std::cout << "ERROR::FILESYSTEM: Failed to find project directory!"
              << std::endl;
    return false;
  }
  return true;
}
