#ifndef GRAPH_H
#define GRAPH_H

#include "LinkedList.h"

class Graph {
private:
    //This graph is an array containing all service ids, each have a linked list of their dependencies
    LinkedList<long long>* parentList;
    long long n;

public:
    //constructor, destructor
    Graph(long long max);
    ~Graph();
    //add and get dependends
    void addDependency(long long parentId, long long dependentId);
    void addDependency(long long parentId, long long* dependentIds, int numDependents);
    LinkedList<long long>& getDependents(long long id);
    //other getter(s)
    long long getCapacity() const {return n;}
};

#endif