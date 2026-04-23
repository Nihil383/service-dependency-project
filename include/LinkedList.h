#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <iostream>
#include <string>

class LinkedList {
private:
    struct Node {
        std::string data;
        Node* next;
        Node(const std::string& value) : data(value), next(nullptr) {}
    };
    
    Node* head;
    Node* tail;
    int listSize;
    
public:
    LinkedList();
    ~LinkedList();
    
    // For Queue operations 
    void push_back(const std::string& value);
    bool pop_front();
    bool empty() const;
    
    // For Status tracking
    bool contains(const std::string& value) const;
    void remove(const std::string& value);
    void clear();
    
    // Utility
    int size() const;
    void print() const;
};

#endif
