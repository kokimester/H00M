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


// https://stackoverflow.com/questions/8515198/basic-aabb-collision-using-projection-vector?utm_source=chatgpt.com
glm::vec3 getCollisionDirection(const AABB& a,const AABB& b){
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

    // If you want touching to count, clamp tiny negatives to zero
    ox = std::max(0.0f, ox);
    oy = std::max(0.0f, oy);
    oz = std::max(0.0f, oz);

    //if you want collision depth return ox,oy,or oz as well depending on
    //which is the smallest

    auto normal = glm::vec3{0.f};
    // Pick smallest overlap
    if (ox <= oy && ox <= oz) {
        normal = glm::vec3((d.x >= 0.0f) ? 1.0f : -1.0f, 0.0f, 0.0f);
    } else if (oy <= oz) {
        normal = glm::vec3(0.0f, (d.y >= 0.0f) ? 1.0f : -1.0f, 0.0f);
    } else {
        normal = glm::vec3(0.0f, 0.0f, (d.z >= 0.0f) ? 1.0f : -1.0f);
    }
    return normal;
}