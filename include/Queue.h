#ifndef QUEUE_H
#define QUEUE_H

#include "LinkedList.h"
#include <cstddef>

// Queue Data Structure
// FIFO
// Implemented using LinkedList
// Used for BFS traversal in Service Dependency Failure Simulator

template<typename T>
class Queue {
private:
    LinkedList<T> list;   // Underlying linked list

public:

    // Constructor
    Queue();

    // Adds an element to the rear of the queue
    void enqueue(const T& value);

    // Removes the front element of the queue
    bool dequeue();

    // Returns the front element of the queue
    T& front();

    // Checks if queue is empty
    bool empty() const;

    // Returns number of elements in queue
    size_t size() const;
};

#include "Queue.cpp"

#endif
