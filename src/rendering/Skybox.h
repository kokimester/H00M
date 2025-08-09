#pragma once

#include "Shader.h"
#include <filesystem>
#include "glm/glm.hpp"

class Skybox{
    public:
    Skybox(Shader& skyboxShader, const std::filesystem::path& skyboxPath, std::array<std::string_view, 6> skyboxCubemapFacesFileNames);
    void render();
    private:

    unsigned int m_VAO;
    unsigned int m_TextureID;
    Shader& m_Shader;
};