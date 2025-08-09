#pragma once
#include "Camera.h"
#include "Entity.h"
#include <cstddef>

class Player {
public:
  Player(EntityType entityID, registry& reg, Camera& camera);

  const Camera& getCamera() const { return m_PlayerCamera; }

  void handleMouseInput(float xChange, float yChange);
  void handleKeyboardInput(bool* keys, float deltaTime);

  void update(float deltaTime);

private:
  enum class PLAYER_STATE {
    STANDING,
    JUMPING
  };
  PLAYER_STATE m_PlayerState = PLAYER_STATE::STANDING;

  
  EntityType m_EntityID;
  registry& m_Registry;
  Camera& m_PlayerCamera;
};
