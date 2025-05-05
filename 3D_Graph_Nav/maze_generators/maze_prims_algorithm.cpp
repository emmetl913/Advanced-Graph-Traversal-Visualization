//
// Created by raven on 5/5/2025.
//
#include "maze_prims_algorithm.h"

std::vector<std::pair<int, int>> PrimsMaze::new_frontier_points() {
   std::vector<std::pair<int,int>> frontier_points;
   frontier_points.emplace_back(current_pos);
   return frontier_points;
}

