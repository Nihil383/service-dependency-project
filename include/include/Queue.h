#ifndef QUEUE_H
#define QUEUE_H

// Queue Data Structure
// Used for BFS traversal in Service Dependency Failure Simulator

template<typename T>
class Queue {
private:

public:
    Queue();
    void enqueue(const T& value);
    bool dequeue();
    bool empty() const;
};

#endif
