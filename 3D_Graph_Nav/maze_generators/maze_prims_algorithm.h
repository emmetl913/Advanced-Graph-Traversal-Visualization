//
// Created by raven on 5/5/2025.
//

#ifndef ADVANCED_GRAPH_TRAVERSAL_VISUALIZATION_MAZE_PRIMS_ALGORITHM_H
#define ADVANCED_GRAPH_TRAVERSAL_VISUALIZATION_MAZE_PRIMS_ALGORITHM_H
#include "maze.h"
#include <unordered_set>
//For use with the unordered set! this will give us very quick computation because
//unordered set has O(1) access time. (Hash)
struct pair_hash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2>& pair) const {
        return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};
class PrimsMaze : public Maze{
private:
    //This function will save its wall breaking history in the wall_history vector that it inherits
    //Generates a maze using Prim's Algorithm
    void generate_maze() override;
    std::vector<std::pair<int,int>> new_frontier_points();
    std::unordered_set<std::pair<int, int>, pair_hash> frontier;
public:
    PrimsMaze(int startX, int startY, int graphWidth, int graphHeight)
            : Maze(startX, startY, graphWidth, graphHeight) {}
};
#endif //ADVANCED_GRAPH_TRAVERSAL_VISUALIZATION_MAZE_PRIMS_ALGORITHM_H
