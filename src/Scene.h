#ifndef SCENE_H
#define SCENE_H

#include "Geometry.h"

struct Scene {
  std::vector<Geometry *> geometries;

  void add(Geometry *g) { geometries.push_back(g); }

  void intersect_all() {
    for (auto g : geometries) {
      for (auto other_g : geometries) {
        if (g == other_g)
          continue;
        if (g->intersects(*other_g))
          std::cout << "intersect!\n";
      }
    }
  }
};

#endif