//
// Created by raven on 5/5/2025.
//

#ifndef ADVANCED_GRAPH_TRAVERSAL_VISUALIZATION_MAZE_PRIMS_ALGORITHM_H
#define ADVANCED_GRAPH_TRAVERSAL_VISUALIZATION_MAZE_PRIMS_ALGORITHM_H
#include "maze.h"
#include <utility>
#include <set>
#include <queue>

class PrimsMaze : public Maze{
private:
    //This function will save its wall breaking history in the wall_history vector that it inherits
    //Generates a maze using Prim's Algorithm
    void generate_maze() override;

    //Stores the locations of all initial rooms
    std::set<std::pair<int,int>> room_set;

    //This is our path.
    std::set<std::pair<int,int>> path_set;

    //Iterate over this to break walls.
    std::vector<std::pair<int,int>> wall_list;

    //Keep track of which walls are not broken. We remove from this set when we break (open) a wall
    std::set<std::pair<int,int>> wall_set;


    //Fill grid with single value (0 = vanilla prim [we can make an ENUM for these if we get that far])
    void set_grid(int grid_coloring);
    bool is_pair_in_set(const std::set<std::pair<int, int>>& my_set, int x, int y);
    std::vector<std::pair<int,int>> get_adjacent_walls(int x, int y);
    std::vector<std::pair<int,int>> get_adjacent_rooms(int x, int y);
    std::pair<int,int> exactly_one_room_not_in_path_adjacent_to_wall(const std::vector<std::pair<int,int>> &adj_rooms);


public:
    PrimsMaze(int startX, int startY, int graphWidth, int graphHeight)
            : Maze(startX, startY, graphWidth, graphHeight) {
            PrimsMaze::generate_maze();
    }
    std::pair<int,int> player_start, goal;
    std::queue<std::pair<std::pair<int,int>, int>> maze_generation_history;

};
#endif //ADVANCED_GRAPH_TRAVERSAL_VISUALIZATION_MAZE_PRIMS_ALGORITHM_H
