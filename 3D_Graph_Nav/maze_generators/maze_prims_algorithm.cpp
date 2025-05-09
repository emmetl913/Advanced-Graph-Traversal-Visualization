//
// Created by raven on 5/5/2025.
//
#include "maze_prims_algorithm.h"
#include <set>
#include <utility>
#include <random>
#include <algorithm>

std::pair<int, int> get_last_item(const std::set<std::pair<int, int>>& path_set) {
    if (!path_set.empty()) {
        return *std::prev(path_set.end());
    }
    return {-1, -1}; // Return a default value if the set is empty
}
std::vector<std::pair<int, int>> PrimsMaze::get_adjacent_walls(int x, int y) {
    std::vector<std::pair<int,int>> neighbors;

    //Gather all neighbors
    //0 == left, 1 == up, 2 == right, 3 == down
    //ensure that each wall is a valid neighbor.
    //the wall must not be our outer border and
    //the position must be a False (closed) wall in the Wall Set
    if(x-1 > 0 && is_pair_in_set(wall_set, x-1, y))
        neighbors.emplace_back(x-1,y);
    if(y+1 < graphHeight*2 - 2 && is_pair_in_set(wall_set, x, y+1))
        neighbors.emplace_back(x,y+1);
    if(x + 1 < graphWidth*2 - 2 && is_pair_in_set(wall_set, x + 1, y))
        neighbors.emplace_back(x+1,y);
    if(y-1 > 0 && is_pair_in_set(wall_set, x, y-1))
        neighbors.emplace_back(x,y-1);

    return neighbors;
}


std::vector<std::pair<int, int>> PrimsMaze::get_adjacent_rooms(int x, int y) {
    std::vector<std::pair<int, int>> rooms;
    //Check left, right, up, and down
    if(x-1 > 0 && is_pair_in_set(room_set, x-1, y))
        rooms.emplace_back(x-1,y);
    if(y+1 < graphHeight*2 - 2 && is_pair_in_set(room_set, x, y+1))
        rooms.emplace_back(x,y+1);
    if(x + 1 < graphWidth*2 - 2 && is_pair_in_set(room_set, x + 1, y))
        rooms.emplace_back(x+1,y);
    if(y-1 > 0 && is_pair_in_set(room_set, x, y-1))
        rooms.emplace_back(x,y-1);
    return rooms;
}
//Wall = 1, Pillar = (1 as a wall - 2 as a different color for debug), Room = 3
void PrimsMaze::set_grid(int grid_coloring) {
    for (int i = 0; i < graphHeight; ++i) {  // -2 is for the border walls
            for (int j = 0; j < graphWidth ; ++j) { // -2 is for the border walls
                if (grid_coloring == 0) {
                    if (i % 2 == 0) {//Even rows go Pillar Wall Pillar
                        if (j % 2 == 0) {
                            grid[i][j] = 1;
                            maze_generation_history.push(std::make_pair(std::make_pair(i,j), 1));
                           // maze_generation_history.emplace_back(std::make_pair(i,j), 1);
                        }
                        else {
                            grid[i][j] = 1;
                            maze_generation_history.push(std::make_pair(std::make_pair(i,j), 1));
                            //Lets add our walls to our wall list
                            wall_set.emplace(i,j);

                        }
                    }
                    else {
                        if (j % 2 == 0) { //Odd rows go Wall, Room, Wall
                            grid[i][j] = 1;
                            maze_generation_history.push(std::make_pair(std::make_pair(i,j), 1));

                            //Lets add our walls to our wall list
                            wall_set.emplace(i,j);

                        }
                        else {
                            grid[i][j] = 0;
                            maze_generation_history.push(std::make_pair(std::make_pair(i,j), 0));

                            //These are rooms. Lets add these to our room set.
                            room_set.emplace(i,j);
                        }
                    }
                }

            }
        }
}

//Returns (-1,-1) if the result is not true, else returns correct wall
std::pair<int,int> PrimsMaze::exactly_one_room_not_in_path_adjacent_to_wall(const std::vector<std::pair<int,int>> &adj_rooms) {
    int count = 0;
    std::pair<int,int> room = std::make_pair(-1,-1);
    for (int i = 0 ; i < adj_rooms.size(); i++) {
        if (!is_pair_in_set(path_set, adj_rooms[i].first, adj_rooms[i].second)) {
            count++;
            room = adj_rooms[i];
        }
    }
    if (count > 1) {
        room = std::make_pair(-1,-1);
    }
    // printf("Found this many not in path adjacent to wall rooms: %i\n", count);
    return room;

}
void PrimsMaze::generate_maze() {
    //1) mark all walls as closed && initialize grid coloring
    set_grid(0);

    //2) Select a room from the set of rooms and add it to the path
    //   For now we will just use the first room in the set
    std::pair<int, int> current_room;
    // path_set.emplace(current_room.first, current_room.second);
    // we can probably save this as the player start
    auto firstItem = *room_set.begin();
    current_room = firstItem;
    path_set.emplace(current_room.first, current_room.second);
    player_start = current_room;
    grid[current_room.first][current_room.second] = 2;
    maze_generation_history.push(std::make_pair(std::make_pair(current_room.first,current_room.second), 2));


    printf("first room: %i, %i\n", firstItem.first, firstItem.second);
    //3) Get the adjacent walls to our first room and add them to the wall list
    std::vector<std::pair<int, int>> adj_walls = get_adjacent_walls(current_room.first, current_room.second);
    for (int i = 0; i < adj_walls.size(); i++) {
        wall_list.emplace_back(adj_walls[i]);
        // printf("added an adj_wall\n");
    }

    //4) While the wall list is not empty, iterate
    while (!wall_list.empty()) {
        // printf("we made it to the loop!\n");
        //a) Select a wall
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, wall_list.size() - 1);
        int random_index = dis(gen);
        std::pair<int, int> current_wall = wall_list[random_index];
        //b) Find adjacent rooms to wall
        std::vector<std::pair<int, int>> adj_rooms = get_adjacent_rooms(current_wall.first, current_wall.second);
        //c) if adj_rooms size == 2, and EXACTLY one room is not in the path_set
        if (adj_rooms.size() == 2) {
            // printf("We found adjacent rooms to our wall\n");
            //count adj_rooms in the path set and if only one is NOT in the path set, return that room else return (-1,-1)
            std::pair<int, int> exact_room = exactly_one_room_not_in_path_adjacent_to_wall(adj_rooms);
            if (exact_room != std::make_pair(-1,-1)) { //Then we have exactly one adj room not in the path set.
                //i) Mark the wall as "Open".
                //maze_generation_history.emplace_back(std::make_pair(current_wall.first,current_wall.second), 0);
                grid[current_wall.first][current_wall.second] = 0;
                maze_generation_history.push(std::make_pair(std::make_pair(current_wall.first,current_wall.second), 0));

                // printf("We broke a wall!\n");

                //ii) Add the unvisited room to the path.
                path_set.emplace(exact_room.first, exact_room.second);

                //iii) Add the walls adjacent to the unvisited room to the wall list.
                adj_walls = get_adjacent_walls(exact_room.first, exact_room.second);
                for(int i = 0; i < adj_walls.size(); i++) {
                    wall_list.emplace_back(adj_walls[i]);
                }
            }
        }
        //d) remove the wall from the wall_list and the wall_set
        wall_set.erase({current_wall.first, current_wall.second});
        wall_list.erase(std::remove(wall_list.begin(), wall_list.end(), std::make_pair(current_wall.first, current_wall.second)), wall_list.end());
        // wall_list.erase(std::remove(wall_list.begin(), wall_list.end(), std::make_pair(current_wall.first, current_wall.second)), wall_list.end());
    }
    std::pair<int, int> last_room = get_last_item(path_set);
    goal = last_room;
    grid[goal.first][goal.second] = 3;
    maze_generation_history.push(std::make_pair(std::make_pair(goal.first, goal.second), 3));

    // for (int i = 0; i < graphHeight; ++i) {
    //     for (int j = 0; j < graphWidth; ++j) {
    //         printf("%i ", grid[i][j]);
    //     }
    //     printf("\n");
    // }
}

bool PrimsMaze::is_pair_in_set(const std::set<std::pair<int, int>>& my_set, int x, int y) {
    return my_set.find({x, y}) != my_set.end();
}