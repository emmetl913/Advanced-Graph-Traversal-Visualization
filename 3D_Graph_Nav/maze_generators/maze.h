//
// Created by raven on 5/5/2025.
//

#ifndef ADVANCED_GRAPH_TRAVERSAL_VISUALIZATION_MAZE_H
#define ADVANCED_GRAPH_TRAVERSAL_VISUALIZATION_MAZE_H

#include <vector>
#include <utility>
class Maze{
public:
    //We need to know where we should begin the algorithm. This will be the player's starting position
    int startX, startY;

    //We need to know the size of our maze. So we take care of that in our constructor with these variables
    int graphWidth, graphHeight;
    int** grid;

    //Lets keep track of each wall we place that way we can draw them dynamically for cool visual effect
    std::vector<std::pair<int,int>> wall_history;

    //Use this point to track our current position in the iteration
    std::pair<int,int> current_pos;

    Maze(int startX, int startY, int graphWidth, int graphHeight)
            : startX(startX), startY(startY), graphWidth(graphWidth), graphHeight(graphHeight)  {
        //Initialize our grid
        grid = new int*[graphHeight * 2];
        for(int i = 0; i < graphHeight * 2; i++){
            grid[i] = new int[graphWidth * 2]();
        }
    }
    //Fill grid with single value
    void set_grid(int value) const{
        for (int i = 0; i < graphHeight*2; ++i) {
            for (int j = 0; j < graphWidth*2; ++j) {
                grid[i][j] = value;
            }
        }
    }
    virtual void generate_maze() = 0; //Override in children class to solve for maze

    //Later we will figure out if we want to redraw wall placement in this class or do elsewhere


    ~Maze(){
        for (int i = 0; i < graphHeight * 2; ++i) {
            delete[] grid[i]; // Delete each row
        }
        delete[] grid; // Delete the array of pointers
    }

};
#endif //ADVANCED_GRAPH_TRAVERSAL_VISUALIZATION_MAZE_H
