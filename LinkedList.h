#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <iostream>
#include <string>
using namespace std;

// queue node
class QueueNode {
public:
    long long id;
    QueueNode* next;
    QueueNode(long long val) : id(val), next(nullptr) {}
};

// for LongHashSet chaining
class LongSetNode {
public:
    long long key;
    LongSetNode* next;
    LongSetNode(long long k) : key(k), next(nullptr) {}
};

// for StringHashMap chaining
class StringMapNode {
public:
    string key;
    long long value;
    StringMapNode* next;
    StringMapNode(const string& k, long long v)
        : key(k), value(v), next(nullptr) {
    }
};

// LinkedList — original string linked list, kept for completeness
// Not currently used in the active design but left in so LinkedList.cpp
// compiles cleanly while the project is still being reorganised
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

    void push_back(const std::string& value);
    bool pop_front();
    bool empty() const;
    bool contains(const std::string& value) const;
    void remove(const std::string& value);
    void clear();
    int size() const;
    void print() const;
};

#endif