#include "Graph.h"
#include <iostream>

Graph::Graph(long long maxServices) {
    // store capacity as max+1 so IDs 1..max are all valid indices
    capacity = maxServices + 1;
    adjList = new EdgeList[capacity];
}

Graph::~Graph() {
    delete[] adjList;
}

void Graph::addEdge(long long parentId, long long dependentId, int weight, bool absolute) {
    // guard against out-of-range IDs
    if (parentId < 1 || parentId >= capacity) {
        std::cout << "[Graph] addEdge: parentId " << parentId << " out of range.\n";
        return;
    }
    if (dependentId < 1 || dependentId >= capacity) {
        std::cout << "[Graph] addEdge: dependentId " << dependentId << " out of range.\n";
        return;
    }
    adjList[parentId].push_front(dependentId, weight, absolute);
}

EdgeList& Graph::getDependents(long long id) {
    // caller is responsible for checking bounds before calling
    return adjList[id];
}
