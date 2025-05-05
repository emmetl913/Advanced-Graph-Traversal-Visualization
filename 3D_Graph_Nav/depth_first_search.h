//
// Created by raven on 4/22/2025.
//

#ifndef ADVANCED_GRAPH_TRAVERSAL_VISUALIZATION_DEPTH_FIRST_SEARCH_H
#define ADVANCED_GRAPH_TRAVERSAL_VISUALIZATION_DEPTH_FIRST_SEARCH_H

#include <set>
#include <vector>
#include <queue>

class depth_first_search {
public:
    int startX, startY;
    int graphWidth, graphHeight;
    std::set<std::pair<int, int>> visited_points;
    std::queue<std::pair<int,int>> point_queue;
    int** grid;
    int recursion_limit = 3;
//    int depth = 0;
    depth_first_search(int startX, int startY, int graphWidth, int graphHeight)
    : startX(startX), startY(startY), graphWidth(graphWidth), graphHeight(graphHeight)  {
        grid = new int*[graphHeight * 2];
        for(int i = 0; i < graphHeight * 2; i++){
            grid[i] = new int[graphWidth * 2]();
        }
        set_grid(1);
        print_grid();
    }

    void set_grid(int value);
    void print_grid();
    void traverse(int x, int y, int depth);
    std::vector<std::pair<int, int>> choose_random_neighbors(int x, int y);

    ~depth_first_search();

};
#endif //ADVANCED_GRAPH_TRAVERSAL_VISUALIZATION_DEPTH_FIRST_SEARCH_H
