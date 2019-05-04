#include "Geometry.h"

#include <glm/gtx/string_cast.hpp>

void Geometry::draw() {
  VAO.bind();
  glDrawElements(GL_TRIANGLES, indices.size() * 3, GL_UNSIGNED_INT,
                 indices.data());
}

bool Geometry::intersects(Geometry &g) {
  std::vector<glm::vec3> isects;
  glm::mat4 transformation = translation * glm::mat4_cast(orientation);

  for (auto self_face : indices) {
    for (auto other_face : g.indices) {
      // intersect two triangles
      glm::vec3 v1 =
          glm::vec3(transformation * glm::vec4(vertices[self_face[0]], 1));
      glm::vec3 v2 =
          glm::vec3(transformation * glm::vec4(vertices[self_face[1]], 1));
      glm::vec3 v3 =
          glm::vec3(transformation * glm::vec4(vertices[self_face[2]], 1));
      glm::vec3 v4 =
          glm::vec3(transformation * glm::vec4(g.vertices[other_face[0]], 1));
      glm::vec3 v5 =
          glm::vec3(transformation * glm::vec4(g.vertices[other_face[1]], 1));
      glm::vec3 v6 =
          glm::vec3(transformation * glm::vec4(g.vertices[other_face[2]], 1));

      bool hit = intersect_triangles(v1, v2, v3, v4, v5, v6);
      if (hit) {
        isects.emplace_back((v1 + v2 + v3) / 3.0f);
        break;
      }
    }
  }
  // print results
  if (!isects.empty()) {
    for (auto b : isects) {
      std::cout << glm::to_string(b) << std::endl;
    }
  }
  std::cout << "AHHH" << std::endl;
  return !isects.empty();
}

float Geometry::vec2_cross(glm::vec2 v1, glm::vec2 v2) {
  return v1.x * v2.y - v1.x * v2.x;
}

bool Geometry::edge_intersect(std::vector<glm::vec2> e1,
                              std::vector<glm::vec2> e2) {

  // std::cout << "e1: " << glm::to_string(e1[0]) << ", " <<
  // glm::to_string(e1[1])
  //           << std::endl;
  // std::cout << "e2: " << glm::to_string(e2[0]) << ", " <<
  // glm::to_string(e2[1])
  //           << std::endl;

  // std::cout << "---------------------------" << std::endl;

  glm::vec2 r = e1[1];
  glm::vec2 s = e2[1];
  glm::vec2 p = e1[0];
  glm::vec2 q = e2[0];

  float r_cross_s = vec2_cross(r, s);
  float q_p_cross_r = vec2_cross(q - p, r);

  float t = vec2_cross((q - p), s) / r_cross_s;
  float u = vec2_cross((q - p), r) / r_cross_s;

  float mr = glm::length(r);
  float ms = glm::length(s);

  if (r_cross_s == 0 && q_p_cross_r == 0) {
    float t0 = glm::dot(q - p, r) / glm::dot(r, r);
    float t1 = t0 + glm::dot(s, r) / glm::dot(r, r);
    float min_rng = glm::min(t0, t1);
    float max_rng = glm::max(t0, t1);
    return glm::max(0.0f, min_rng) <= glm::min(1.0f, max_rng);
  } else if (r_cross_s == 0 && q_p_cross_r != 0) {
    return false;
  } else if (r_cross_s != 0 && 0 <= t && t <= 1 && 0 <= u && u <= 1) {
    return true;
  }
  return false;
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

  // if the triangles are coplanar
  if (!(t1_not_on_plane || t2_not_on_plane)) {

    // choose best axis
    glm::vec3 N1_normalized = glm::normalize(glm::abs(N1));
    float x_diff = glm::distance(glm::vec3(1, 0, 0), N1_normalized);
    float y_diff = glm::distance(glm::vec3(0, 1, 0), N1_normalized);
    float z_diff = glm::distance(glm::vec3(0, 0, 1), N1_normalized);

    float best_diff = glm::min(x_diff, glm::min(y_diff, z_diff));
    int x_swizzle = -1;
    int y_swizzle = -1;
    glm::vec3 best_axis;
    if (x_diff == best_diff) {
      best_axis = glm::vec3(1, 0, 0);
      x_swizzle = 1;
      y_swizzle = 2;
    } else if (y_diff == best_diff) {
      best_axis = glm::vec3(0, 1, 0);
      x_swizzle = 0;
      y_swizzle = 2;
    } else if (z_diff == best_diff) {
      best_axis = glm::vec3(0, 0, 1);
      x_swizzle = 0;
      y_swizzle = 1;
    } else {
      std::cerr << "::ERROR - no axis aligned" << std::endl;
    }

    // project points onto new plane
    glm::vec3 p31 = v1 - glm::dot(best_axis, v1) * best_axis;
    glm::vec3 p32 = v2 - glm::dot(best_axis, v2) * best_axis;
    glm::vec3 p33 = v3 - glm::dot(best_axis, v3) * best_axis;

    glm::vec3 p34 = v4 - glm::dot(best_axis, v4) * best_axis;
    glm::vec3 p35 = v5 - glm::dot(best_axis, v5) * best_axis;
    glm::vec3 p36 = v6 - glm::dot(best_axis, v6) * best_axis;

    // swizzle to get projected 2d coordinates
    glm::vec2 p1 = glm::vec2(p31[x_swizzle], p31[y_swizzle]);
    glm::vec2 p2 = glm::vec2(p32[x_swizzle], p32[y_swizzle]);
    glm::vec2 p3 = glm::vec2(p33[x_swizzle], p33[y_swizzle]);

    glm::vec2 p4 = glm::vec2(p34[x_swizzle], p34[y_swizzle]);
    glm::vec2 p5 = glm::vec2(p35[x_swizzle], p35[y_swizzle]);
    glm::vec2 p6 = glm::vec2(p36[x_swizzle], p36[y_swizzle]);

    std::vector<std::vector<glm::vec2>> edges_1 = {
        {p1, p2}, {p2, p3}, {p3, p1}};
    std::vector<std::vector<glm::vec2>> edges_2 = {
        {p4, p5}, {p5, p6}, {p6, p4}};
    for (auto e_1 : edges_1) {
      for (auto e_2 : edges_2) {
        bool e_intersect = edge_intersect(e_1, e_2); //, vec2_i);
        if (e_intersect)
          return true;
      }
    }
  }
  // if the triangles are not coplanar
  else {
    glm::vec3 D = glm::normalize(glm::cross(N1, N2));

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
    if (glm::isnan(t1) || glm::isnan(t2) || glm::isnan(t3) || glm::isnan(t4))
      return false;
    if (glm::max(t1, t3) < glm::min(t2, t4))
      return true;
  }

  return false;
}