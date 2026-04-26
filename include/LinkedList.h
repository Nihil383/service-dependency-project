#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <iostream>
template <typename T>
class LinkedList {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& value) : data(value), next(nullptr) {}};
    Node* head;
    Node* tail;
    int listSize;
public:
    // Iterator class
    class Iterator {
    private:
        Node* current;
    public:
        Iterator(Node* node) : current(node) {}
        T& operator*() { return current->data; }
        Iterator& operator++() {
            if (current) current = current->next;
            return *this;
        }
        bool operator!=(const Iterator& other) const {
            return current != other.current;
        }
    };
    
    // Constructors & Destructor
    LinkedList() : head(nullptr), tail(nullptr), listSize(0) {}
    ~LinkedList() { clear(); }
    
    // Core functions needed
    void push_back(const T& value);
    bool pop_front();
    bool empty() const;
    int size() const;
    void clear();
    
    // Iterators
    Iterator begin() { return Iterator(head); }
    Iterator end() { return Iterator(nullptr); }
};

#include "LinkedList.cpp"
#endif
