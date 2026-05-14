#include "Graph.h"
#include "Vector.h"
#include <iostream>

Graph::Graph() : serviceCount(0) {
    // Vector starts empty - no max size needed
    adjList = new Vector();
    // reserve index 0 as unused so service IDs start at 1
    adjList->grow();
}

Graph::~Graph() {
    delete adjList;
}

// registerService - grow the vector by one slot, return the new index
long long Graph::registerService() {
    long long idx = adjList->grow();
    serviceCount++;
    return idx;
}

// addEdge - insert edge only if it doesn't already exist
bool Graph::addEdge(long long parentId, long long dependentId,
                    int weight, bool absolute) {
    if (parentId < 1 || parentId >= adjList->size()) {
        std::cout << "[Graph] addEdge: parentId " << parentId << " out of range.\n";
        return false;
    }
    if (dependentId < 1 || dependentId >= adjList->size()) {
        std::cout << "[Graph] addEdge: dependentId " << dependentId << " out of range.\n";
        return false;
    }

    // duplicate check - prevents re-adding the same edge from UI or file
    if ((*adjList)[parentId].edgeExists(dependentId)) {
        std::cout << "[Graph] addEdge: edge " << parentId
                  << " -> " << dependentId << " already exists.\n";
        return false;
    }

    (*adjList)[parentId].push_front(dependentId, weight, absolute);
    return true;
}

EdgeList& Graph::getDependents(long long id) {
    return (*adjList)[id];
}

const EdgeList& Graph::getEdgesFrom(long long id) const {
    return (*adjList)[id];
}
