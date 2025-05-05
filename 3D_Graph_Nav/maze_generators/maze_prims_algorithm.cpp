//
// Created by raven on 5/5/2025.
//
#include "maze_prims_algorithm.h"

std::vector<std::pair<int, int>> PrimsMaze::new_frontier_points() {
    std::vector<std::pair<int,int>> neighbors;
    int x = current_pos.first, y = current_pos.second;

    //Gather all neighbors
    //0 == left, 1 == up, 2 == right, 3 == down
    //ensure that each wall is a valid neighbor.
    //the wall must not be our outer border and
    //the position must be a wall not in the frontier already
    // (set ensures no duplicates) - don't need to check in our code
    if(x-1 > 0)
        neighbors.emplace_back(x-1,y);
    if(y+1 < graphHeight*2 - 2)
        neighbors.emplace_back(x,y+1);
    if(x + 1 < graphWidth*2 - 2)
        neighbors.emplace_back(x+1,y);
    if(y-1 > 0)
        neighbors.emplace_back(x,y-1);

    return neighbors;
}

void PrimsMaze::generate_maze() {
    //Initialize grid as walls
    set_grid(1);

    //Add neighboring tiles to the frontier list.
    std::vector<std::pair<int, int>> l = new_frontier_points();
    for(auto & i : l){
        frontier.insert(i);
    }
    /* While the frontier list is not empty:
               Randomly select a wall tile from the frontier list.
               Check if the wall tile separates a passage from another wall tile:
                   If true, convert the wall tile into a passage (value 0).
                   Also, convert the wall tile's neighboring wall (in the direction of the passage) into a passage.
               Add all neighboring wall tiles of the newly created passage to the frontier list (if they are not already in the list)
    */
    while(!frontier.empty()){

    }


}

