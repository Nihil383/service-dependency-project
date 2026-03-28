#ifndef LINKEDLIST_H
#define LINKEDLIST_H

template<typename T>
class LinkedList {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& value) : data(value), next(nullptr) {}
    };
    
    Node* head;
    size_t listSize;
    
public:
    LinkedList();
    LinkedList(const LinkedList& other);
    ~LinkedList();
    LinkedList& operator=(const LinkedList& other);
    
    void push_front(const T& value);
    void push_back(const T& value);
    bool pop_front();
    bool remove(const T& value);
    bool contains(const T& value) const;
    size_t size() const;
    bool empty() const;
    void clear();
    void print() const;
    
    class Iterator {
    private:
        Node* current;
    public:
        Iterator(Node* node) : current(node) {}
        T& operator*() { return current->data; }
        Iterator& operator++() { if (current) current = current->next; return *this; }
        bool operator!=(const Iterator& other) const { return current != other.current; }
    };
    
    Iterator begin() { return Iterator(head); }
    Iterator end() { return Iterator(nullptr); }
};

#include "LinkedList.cpp"
#endif