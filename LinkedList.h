#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <iostream>
#include <string>
using namespace std;

// -----------------------------------------------------------------------
// Node types used by the hash structures and queue
// -----------------------------------------------------------------------

// Used by IDQueue
class QueueNode {
public:
    long long id;
    QueueNode* next;
    QueueNode(long long val) : id(val), next(nullptr) {}
};

// Used by LongHashSet for chaining
class LongSetNode {
public:
    long long key;
    LongSetNode* next;
    LongSetNode(long long k) : key(k), next(nullptr) {}
};

// Used by StringHashMap for chaining
class StringMapNode {
public:
    string key;
    long long value;
    StringMapNode* next;
    StringMapNode(const string& k, long long v)
        : key(k), value(v), next(nullptr) {}
};

// -----------------------------------------------------------------------
// IDListNode / IDList
// Lightweight singly-linked list of long long service IDs.
// Used by ServiceManager to maintain persistent normal/affected/failed
// lists that update in O(1) insert and O(n-bucket) remove per status
// change, so filtered queries never need to scan the whole service array.
// -----------------------------------------------------------------------
struct IDListNode {
    long long   id;
    IDListNode* next;
    IDListNode(long long val) : id(val), next(nullptr) {}
};

class IDList {
private:
    IDListNode* head;
    int         listSize;

public:
    IDList() : head(nullptr), listSize(0) {}
    ~IDList() { clear(); }

    // pushFront - O(1), order within a status list doesn't matter
    void pushFront(long long id) {
        IDListNode* node = new IDListNode(id);
        node->next = head;
        head       = node;
        listSize++;
    }

    // remove - O(n) worst case but status lists stay small in practice
    void remove(long long id) {
        IDListNode* curr = head;
        IDListNode* prev = nullptr;
        while (curr) {
            if (curr->id == id) {
                if (prev) prev->next = curr->next;
                else      head       = curr->next;
                delete curr;
                listSize--;
                return;
            }
            prev = curr;
            curr = curr->next;
        }
    }

    void clear() {
        while (head) {
            IDListNode* temp = head;
            head = head->next;
            delete temp;
        }
        listSize = 0;
    }

    IDListNode* getHead() const { return head; }
    int         size()    const { return listSize; }
    bool        empty()   const { return head == nullptr; }
};

// -----------------------------------------------------------------------
// LinkedList - original string linked list, kept so LinkedList.cpp
// compiles cleanly while the project is being reorganised
// -----------------------------------------------------------------------
class LinkedList {
private:
    struct Node {
        std::string data;
        Node*       next;
        Node(const std::string& value) : data(value), next(nullptr) {}
    };

    Node* head;
    Node* tail;
    int   listSize;

public:
    LinkedList();
    ~LinkedList();

    void push_back(const std::string& value);
    bool pop_front();
    bool empty() const;
    bool contains(const std::string& value) const;
    void remove(const std::string& value);
    void clear();
    int  size() const;
    void print() const;
};

#endif
