#include "LinkedList.h"

LinkedList::LinkedList() : head(nullptr), tail(nullptr), listSize(0) {}

LinkedList::~LinkedList() {
    clear();
}

// Add to end (enqueue)
void LinkedList::push_back(const std::string& value) {
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

//  Remove from front (dequeue)
bool LinkedList::pop_front() {
    if (!head) return false;
    
    Node* temp = head;
    head = head->next;
    
    if (!head) {
        tail = nullptr;
    }
    
    delete temp;
    listSize--;
    return true;
}

bool LinkedList::empty() const {
    return head == nullptr;
}
bool LinkedList::contains(const std::string& value) const {
    Node* current = head;
    while (current) {
        if (current->data == value) return true;
        current = current->next;
    }
    return false;
}

// Remove service from list
void LinkedList::remove(const std::string& value) {
    if (!head) return;
    
    // Remove from head
    if (head->data == value) {
        Node* temp = head;
        head = head->next;
        
        if (!head) {
            tail = nullptr;
        }
        
        delete temp;
        listSize--;
        return;
    }
    
    // Remove from middle or end
    Node* current = head;
    while (current->next && current->next->data != value) {
        current = current->next;
    }
    
    if (current->next) {
        Node* temp = current->next;
        current->next = temp->next;
        
        if (!current->next) {
            tail = current;
        }
        
        delete temp;
        listSize--;
    }
}

// Reset all services
void LinkedList::clear() {
    while (head) {
        Node* temp = head;
        head = head->next;
        delete temp;
    }
    tail = nullptr;
    listSize = 0;
}

int LinkedList::size() const {
    return listSize;
}

void LinkedList::print() const {
    Node* current = head;
    std::cout << "[";
    while (current) {
        std::cout << current->data;
        if (current->next) std::cout << ", ";
        current = current->next;
    }
    std::cout << "]";
}
