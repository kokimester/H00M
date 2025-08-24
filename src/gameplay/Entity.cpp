#include "Entity.h"


bool doesCollideAABBvAABB(const AABB& a,const AABB& b){
    return
      a.min.x <= b.max.x &&
      a.max.x >= b.min.x &&
      a.min.y <= b.max.y &&
      a.max.y >= b.min.y &&
      a.min.z <= b.max.z &&
      a.max.z >= b.min.z;
}

bool doesCollidePointvAABB(const glm::vec3& point, const AABB& box){
  return
    point.x >= box.min.x &&
    point.x <= box.max.x &&
    point.y >= box.min.y &&
    point.y <= box.max.y &&
    point.z >= box.min.z &&
    point.z <= box.max.z;
}


glm::vec3 getCollisionDirection(const glm::vec3& a, const glm::vec3& b){
  auto dir = a-b;
  int directionIndex = 0;
  for(int i = 0; i < 3; ++i){
    if(std::abs(dir[i]) > std::abs(dir[directionIndex])){
      directionIndex = i;
    }
  }
  glm::vec3 retVal = glm::vec3{0.f};
  retVal[directionIndex] = dir[directionIndex] > 0.f ? 1.f : -1.f;
  return retVal;
}
// https://stackoverflow.com/questions/8515198/basic-aabb-collision-using-projection-vector?utm_source=chatgpt.com
glm::vec3 getCollisionDirection(const AABB& a,const AABB& b){
  glm::vec3 centerA = (a.max + a.min) / 2.0f;
  glm::vec3 centerB = (b.max + b.min) / 2.0f;
  return getCollisionDirection(centerA,centerB);
}

//chatgpt
// Returns a real collision normal and minimal penetration depth.
// Works well even when boxes are only slightly overlapping.
AABBManifold collideAABBvAABB_Manifold(const AABB& aIn, const AABB& bIn, float EPS) {
    // AABB a = normalizeAABB(aIn);
    // AABB b = normalizeAABB(bIn);
    auto a = aIn;
    auto b = bIn;

    // Centers and half-extents
    const glm::vec3 ca = 0.5f * (a.min + a.max);
    const glm::vec3 cb = 0.5f * (b.min + b.max);
    const glm::vec3 ha = 0.5f * (a.max - a.min);
    const glm::vec3 hb = 0.5f * (b.max - b.min);

    const glm::vec3 d = cb - ca; // from A to B

    // Overlap along each axis (sum of half-extents minus center distance)
    float ox = (ha.x + hb.x) - std::abs(d.x);
    float oy = (ha.y + hb.y) - std::abs(d.y);
    float oz = (ha.z + hb.z) - std::abs(d.z);

    AABBManifold m;

    // If any axis is separated (negative overlap), there is no collision
    if (ox <= -EPS || oy <= -EPS || oz <= -EPS) {
        m.colliding = false;
        return m;
    }

    // Choose the axis of minimum penetration depth
    // (the smallest positive overlap is the MTD axis)
    m.colliding = (ox >= -EPS && oy >= -EPS && oz >= -EPS);

    // If you want touching to count, clamp tiny negatives to zero
    ox = std::max(0.0f, ox);
    oy = std::max(0.0f, oy);
    oz = std::max(0.0f, oz);

    // Pick smallest overlap
    if (ox <= oy && ox <= oz) {
        m.depth = ox;
        m.normal = glm::vec3((d.x >= 0.0f) ? 1.0f : -1.0f, 0.0f, 0.0f);
    } else if (oy <= oz) {
        m.depth = oy;
        m.normal = glm::vec3(0.0f, (d.y >= 0.0f) ? 1.0f : -1.0f, 0.0f);
    } else {
        m.depth = oz;
        m.normal = glm::vec3(0.0f, 0.0f, (d.z >= 0.0f) ? 1.0f : -1.0f);
    }

    return m;
}
//chatgpt