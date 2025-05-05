//
// Created by raven on 5/5/2025.
//

#ifndef ADVANCED_GRAPH_TRAVERSAL_VISUALIZATION_MAZE_PRIMS_ALGORITHM_H
#define ADVANCED_GRAPH_TRAVERSAL_VISUALIZATION_MAZE_PRIMS_ALGORITHM_H
#include "maze.h"
class PrimsMaze : public Maze{
private:
    //This function will save its wall breaking history in the wall_history vector that it inherits
    //Generates a maze using Prim's Algorithm
    void generate_maze() override {

    }
    std::vector<std::pair<int,int>> new_frontier_points();

public:
    PrimsMaze(int startX, int startY, int graphWidth, int graphHeight)
            : Maze(startX, startY, graphWidth, graphHeight) {
        set_grid(1);
    }
};
#endif //ADVANCED_GRAPH_TRAVERSAL_VISUALIZATION_MAZE_PRIMS_ALGORITHM_H
