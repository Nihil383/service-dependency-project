#ifndef QUEUE_H
#define QUEUE_H

#include "LinkedList.h"
#include <cstddef>

//v1 templated
// Queue Data Structure
// FIFO
// Implemented using LinkedList
// Used for BFS traversal in Service Dependency Failure Simulator

/*template<typename T>
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
};*/

//v2 not templated
// IDQueue — FIFO queue of long long service IDs
// Used exclusively for BFS traversal in simulateFailure()

class IDQueue {
private:
    QueueNode* head;  // front of queue (dequeue from here)
    QueueNode* tail;  // back of queue  (enqueue here)
    int qSize;

public:
    IDQueue() : head(nullptr), tail(nullptr), qSize(0) {}

    ~IDQueue() {
        // drain the queue on destruction
        while (head) {
            QueueNode* temp = head;
            head = head->next;
            delete temp;
        }
    }

    //add to back
    void enqueue(long long id) {
        QueueNode* node = new QueueNode(id);
        if (!tail) {
            head = tail = node;
        }
        else {
            tail->next = node;
            tail = node;
        }
        qSize++;
    }

    //remove from front
    void dequeue() {
        if (!head) return;
        QueueNode* temp = head;
        head = head->next;
        if (!head) tail = nullptr;
        delete temp;
        qSize--;
    }

    //peek
    long long front() const {
        return head->id;
    }

    bool empty() const {
        return head == nullptr;
    }

    int size() const {
        return qSize;
    }
};

#endif
