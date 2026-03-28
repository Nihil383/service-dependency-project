#include "LinkedList.h"
#include <iostream>

template<typename T>
LinkedList<T>::LinkedList() : head(nullptr), listSize(0) {}

template<typename T>
LinkedList<T>::LinkedList(const LinkedList& other) : head(nullptr), listSize(0) {
    Node* current = other.head;
    while (current) {
        push_back(current->data);
        current = current->next;
    }
}

template<typename T>
LinkedList<T>::~LinkedList() {
    clear();
}

template<typename T>
LinkedList<T>& LinkedList<T>::operator=(const LinkedList& other) {
    if (this != &other) {
        clear();
        Node* current = other.head;
        while (current) {
            push_back(current->data);
            current = current->next;
        }
    }
    return *this;
}

template<typename T>
void LinkedList<T>::push_front(const T& value) {
    Node* newNode = new Node(value);
    newNode->next = head;
    head = newNode;
    listSize++;
}

template<typename T>
void LinkedList<T>::push_back(const T& value) {
    Node* newNode = new Node(value);
    if (!head) {
        head = newNode;
    } else {
        Node* current = head;
        while (current->next) current = current->next;
        current->next = newNode;
    }
    listSize++;
}

template<typename T>
bool LinkedList<T>::pop_front() {
    if (!head) return false;
    Node* temp = head;
    head = head->next;
    delete temp;
    listSize--;
    return true;
}

template<typename T>
bool LinkedList<T>::remove(const T& value) {
    if (!head) return false;
    
    if (head->data == value) {
        Node* temp = head;
        head = head->next;
        delete temp;
        listSize--;
        return true;
    }
    
    Node* current = head;
    while (current->next && current->next->data != value) {
        current = current->next;
    }
    
    if (current->next) {
        Node* temp = current->next;
        current->next = temp->next;
        delete temp;
        listSize--;
        return true;
    }
    
    return false;
}

template<typename T>
bool LinkedList<T>::contains(const T& value) const {
    Node* current = head;
    while (current) {
        if (current->data == value) return true;
        current = current->next;
    }
    return false;
}

template<typename T>
size_t LinkedList<T>::size() const {
    return listSize;
}

template<typename T>
bool LinkedList<T>::empty() const {
    return head == nullptr;
}

template<typename T>
void LinkedList<T>::clear() {
    while (head) {
        Node* temp = head;
        head = head->next;
        delete temp;
    }
    listSize = 0;
}

template<typename T>
void LinkedList<T>::print() const {
    Node* current = head;
    std::cout << "[";
    while (current) {
        std::cout << current->data;
        if (current->next) std::cout << ", ";
        current = current->next;
    }
    std::cout << "]";
}