#ifndef LINKEDLIST_CPP
#define LINKEDLIST_CPP

#include "LinkedList.h"

/*template <typename T>
void LinkedList<T>::push_back(const T& value) {
    try {
        Node* newNode = new Node(value);
        
        if (!head) {
            head = newNode;
            tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        listSize++;
    }
    catch () {
        throw;}}

template <typename T>
bool LinkedList<T>::pop_front() {
    try {
        if (!head) return false;
        
        Node* temp = head;
        head = head->next;
        if (!head) tail = nullptr;
        delete temp;
        listSize--;
        return true;
    }
    catch () {
        throw;}}*/

template <typename T>
bool LinkedList<T>::empty() const {
    return head == nullptr;
}

template <typename T>
int LinkedList<T>::size() const {
    return listSize;
}

template <typename T>
void LinkedList<T>::clear() {
    try {
        while (head) {
            Node* temp = head;
            head = head->next;
            delete temp; }
        tail = nullptr;
        listSize = 0;
    }
    catch () {
        throw;}}

#endif
