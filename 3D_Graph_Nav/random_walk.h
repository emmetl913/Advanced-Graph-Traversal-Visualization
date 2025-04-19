//
// Created by raven on 4/19/2025.
//

#ifndef ADVANCED_GRAPH_TRAVERSAL_VISUALIZATION_RANDOM_WALK_H
#define ADVANCED_GRAPH_TRAVERSAL_VISUALIZATION_RANDOM_WALK_H

#include <deque>

class random_walk {
public:
    int startX, startY;

    // Deque to store history of movements
    // Deque allows us to O(1) access from front and back
    std::deque<std::pair<int, int>> movement_history;

    random_walk(int startX, int startY) : startX(startX), startY(startY) {}


    ~random_walk();



};


#endif //ADVANCED_GRAPH_TRAVERSAL_VISUALIZATION_RANDOM_WALK_H
