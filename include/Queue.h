#ifndef QUEUE_H
#define QUEUE_H

#include "LinkedList.h"

// Queue Data Structure
// Used for BFS traversal in Service Dependency Failure Simulator

template<typename T>
class Queue {
private:
    LinkedList<T> list;

public:
    Queue();

    void enqueue(const T& value);

    bool dequeue();

    T& front();

    bool empty() const;

    size_t size() const;
};

#include "Queue.cpp"

#endif
