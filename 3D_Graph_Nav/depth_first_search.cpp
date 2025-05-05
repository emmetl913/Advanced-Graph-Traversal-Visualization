//
// Created by raven on 4/22/2025.
//


#include "depth_first_search.h"
#include <random>
#include <iostream>

depth_first_search::~depth_first_search(){
    // Deallocate the 2D array
    for (int i = 0; i < graphHeight * 2; ++i) {
        delete[] grid[i];
    }
    delete[] grid;
}
//returns (-1,-1) if the pair was already in the set or is an outer wall/out of bounds
std::vector<std::pair<int,int>> depth_first_search::choose_random_neighbors(int x, int y){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<std::pair<int,int>> neighbors;
    std::vector<std::pair<int,int>> valid_neighbors;

    std::vector<int> indices = {0,1,2,3};
    //0 == left, 1 == up, 2 == right, 3 == downa
    neighbors.emplace_back(x-1,y);
    neighbors.emplace_back(x,y+1);
    neighbors.emplace_back(x+1,y);
    neighbors.emplace_back(x,y-1);
    for (int i = 0; i < 4; i++){
        std::uniform_int_distribution<int> dist(0, (int)indices.size() -1 );
        int r = dist(gen);
        int idx = indices[r];
        indices.erase(indices.begin() + r );
        if(idx == 0){
            //Ensure left boundary is okay and it is not in the set of visited points
            if(neighbors[0].first > 0 && visited_points.find(neighbors[0]) == visited_points.end()){
                valid_neighbors.emplace_back(neighbors[0]);
//                visited_points.insert(neighbors[0]);
            }
        }
        else if (idx == 1) //check up
        {
            if(neighbors[1].second < graphHeight*2 - 2 && visited_points.find(neighbors[1]) == visited_points.end()){
                valid_neighbors.emplace_back(neighbors[1]);
//                visited_points.insert(neighbors[1]);
            }
        }
        else if (idx == 2) //check right
        {
            if(neighbors[2].second < graphWidth*2 - 2 && visited_points.find(neighbors[2]) == visited_points.end()){
                valid_neighbors.emplace_back(neighbors[2]);
                //visited_points.insert(neighbors[2]);
            }
        }
        else if (idx == 3) //check down
        {
            if(neighbors[3].second > 0 && visited_points.find(neighbors[3]) == visited_points.end()){
                valid_neighbors.emplace_back(neighbors[3]);
//                visited_points.insert(neighbors[3]);
            }
        }
    }
    return valid_neighbors;

}
void depth_first_search::traverse(int x, int y, int depth) {
    //First we add our current location to the set.
    //we start with the player position when we run the function
    visited_points.insert(std::make_pair(x,y));
    point_queue.emplace(x,y);
    //Randomly select a valid neighbor.
    std::pair<int,int> current_node = std::make_pair(x,y);
    int iteration = 0;
    while(!point_queue.empty()){
        printf("iteration %i\n", ++iteration);
        //while point queue not empty we need to get a neighbor.
        std::vector<std::pair<int,int>> neighbors = choose_random_neighbors(current_node.first,current_node.second);
        if (!neighbors.empty()){ //if there is a valid neighbor do algorithm
            std::pair<int,int> one_neighbor = neighbors[0];

            printf("current pos: %i,%i | selected neighbor: %i,%i\n", current_node.first,current_node.second, one_neighbor.first, one_neighbor.second);
            //Break the wall between current_node and neighbor
            grid[one_neighbor.first][one_neighbor.second] = 0;
            current_node = one_neighbor;
            point_queue.emplace(current_node);
            visited_points.insert(current_node);

        }
        else{
            //continue to pop items off the queue until a node is encountered with at least one valid, unvisited neighbor
            while(!point_queue.empty()){
                current_node = point_queue.front();
                point_queue.pop();
                neighbors = choose_random_neighbors(current_node.first,current_node.second);
                if (!neighbors.empty()){
                    current_node = neighbors[0];
                    visited_points.insert(current_node);
                }
            }
        }

    }

    printf("Size of set %zu", visited_points.size());

}
//We set every position to 1, except for the player's starting position already.
//We call this in the constructor and pass in 1.
void depth_first_search::set_grid(int value) {
    // Example: Fill the grid with a specific value
    for (int i = 0; i < graphHeight*2; ++i) {
        for (int j = 0; j < graphWidth*2; ++j) {
            grid[i][j] = value;
        }
    }
}

void depth_first_search::print_grid() {
    for (int i = 0; i < graphHeight*2; ++i) {
        for (int j = 0; j < graphWidth*2; ++j) {
            printf("%d ", grid[i][j]);
        }
        printf("\n");
    }
}

