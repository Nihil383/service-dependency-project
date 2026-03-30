#include "Graph.h"

//parentList takes on an array (size inputted at beginning of program as max) of linked lists
Graph::Graph(long long max) {
    n = max + 1; 
    parentList = new LinkedList<long long>[n]; 
}

//deconstructor
Graph::~Graph() {
    delete[] parentList; 
}

//Simple dependency addition, should be O(1) if push_front adds at tail
void Graph::addDependency(long long parentId, long long dependentId) {
    if (parentId >= 0 && parentId < n) {
        parentList[parentId].push_front(dependentId);
    }
}
//Other dependency addition implementation when input is a list of dependends and a parent (to make it easier to add services)
void Graph::addDependency(long long parentId, long long* dependentIds, int num) {
    //nonsense case for parent/ bad array
    if (parentId < 1 || parentId >= n || dependentIds == nullptr) {
        return;
    }
    //adds all dependends
    for (int i = 0; i < num; ++i) {
        //in progress: this can be later further protected if id doesn't match an actual service in use
        if (dependentIds[i] >= 0 && dependentIds[i] < n) {
            parentList[parentId].push_front(dependentIds[i]);
        }
    }
}

// This will later be used to implement the BFS
LinkedList<long long>& Graph::getDependents(long long id) {
    if (id >= 0 && id < n) {
        return parentList[id];
    }
//this needs an error or so
}