#include "Geometry.h"

void Geometry::draw() {
  VAO.bind();
  glDrawElements(GL_TRIANGLES, indices.size() * 3, GL_UNSIGNED_INT,
                 indices.data());
}

bool Geometry::intersects(Geometry &g) {
  bool isects = false;

  for (auto self_face : indices) {
    for (auto other_face : g.indices) {
      // intersect two triangles
      isects = intersect_triangles(
          vertices[self_face[0]], vertices[self_face[1]],
          vertices[self_face[2]], g.vertices[other_face[0]],
          g.vertices[other_face[1]], g.vertices[other_face[2]]);
      if (isects) {
        return true;
      }
    }
  }
  return isects;
}

bool Geometry::intersect_triangles(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3,
                                   glm::vec3 v4, glm::vec3 v5, glm::vec3 v6) {
  // check if T1 is on one side of pi2
  glm::vec3 N2 = glm::cross((v5 - v4), (v6 - v4));
  float d2 = dot(-N2, v4);

  float dv1 = dot(N2, v1) + d2;
  float dv2 = dot(N2, v2) + d2;
  float dv3 = dot(N2, v3) + d2;

  bool t1_not_on_plane = ((dv1 != 0) && (dv2 != 0) && (dv3 != 0));
  bool all_positive = ((dv1 > 0) && (dv2 > 0) && (dv3 > 0));
  bool all_negative = ((dv1 < 0) && (dv2 < 0) && (dv3 < 0));

  if (t1_not_on_plane && (all_positive || all_negative))
    return false;

  // check if T2 is on one side of pi1
  glm::vec3 N1 = glm::cross((v2 - v1), (v3 - v1));
  float d1 = dot(-N1, v1);

  float dv4 = dot(N1, v4) + d1;
  float dv5 = dot(N1, v5) + d1;
  float dv6 = dot(N1, v6) + d1;

  bool t2_not_on_plane = ((dv4 != 0) && (dv5 != 0) && (dv6 != 0));
  all_positive = ((dv4 > 0) && (dv5 > 0) && (dv6 > 0));
  all_negative = ((dv4 < 0) && (dv5 < 0) && (dv6 < 0));

  if (t2_not_on_plane && (all_positive || all_negative))
    return false;

  // if the triangles are not coplanar
  if (t1_not_on_plane && t2_not_on_plane) {
    glm::vec3 D = glm::cross(N1, N2);

    float pv1_1 = glm::dot(D, v1);
    float pv1_2 = glm::dot(D, v2);
    float pv1_3 = glm::dot(D, v3);

    float t1 = pv1_1 + (pv1_2 - pv1_1) * (dv1 / (dv1 - dv2));
    float t2 = pv1_1 + (pv1_3 - pv1_1) * (dv1 / (dv1 - dv3));

    float pv2_1 = glm::dot(D, v4);
    float pv2_2 = glm::dot(D, v5);
    float pv2_3 = glm::dot(D, v6);

    float t3 = pv2_1 + (pv2_2 - pv2_1) * (dv4 / (dv4 - dv5));
    float t4 = pv2_1 + (pv2_3 - pv2_1) * (dv4 / (dv4 - dv6));

    // if intervals overlap
    if (glm::max(t1, t3) <= glm::min(t2, t4))
      return true;

  }
  // if the triangles are coplanar
  else {
  }

  return false;
}