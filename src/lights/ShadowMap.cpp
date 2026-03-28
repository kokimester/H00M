#include "ShadowMap.h"
#include "CommonValues.h"
void ShadowMap::render(model_system& ms, registry& componentRegistry,
                       Shader& shadow_shader, Shader& normal_shader,
                       const glm::mat4& lightSpaceMatrix) {
  assert(depthMapFBO != 0 && "ShadowMap object is uninitialized");
  assert(depthMapTextureID != 0 && "ShadowMap object is uninitialized");
  // 1. first render to depth map
  shadow_shader.setMat4fv(lightSpaceMatrix, "lightSpaceMatrix");
  glCullFace(GL_FRONT);
  glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glClear(GL_DEPTH_BUFFER_BIT);
  shadow_shader.use();
  ms.render(componentRegistry, shadow_shader);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glCullFace(GL_BACK); // don't forget to reset original culling face

  // 2. then render scene as normal with shadow mapping (using depth map)
  glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  normal_shader.setMat4fv(lightSpaceMatrix, "lightSpaceMatrix");
  // TODO: textures IDs need to be managed in a single component
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, depthMapTextureID);
  ms.render(componentRegistry, normal_shader);
}

void ShadowMap::setup() {
  std::println("Setup shadowmap with:");
  std::println("Screen: {} x {}", SCR_WIDTH, SCR_HEIGHT);
  std::println("Shadow: {} x {}", SHADOW_WIDTH, SHADOW_HEIGHT);
  glGenFramebuffers(1, &depthMapFBO);
  glGenTextures(1, &depthMapTextureID);
  glBindTexture(GL_TEXTURE_2D, depthMapTextureID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH,
               SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         depthMapTextureID, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMap::clear() {
  depthMapFBO       = 0;
  depthMapTextureID = 0;
}
