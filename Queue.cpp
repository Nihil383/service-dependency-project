#include "Queue.h"

template<typename T>
Queue<T>::Queue() {}

template<typename T>
void Queue<T>::enqueue(const T& value) {
    list.push_back(value);
}

template<typename T>
bool Queue<T>::dequeue() {
    return list.pop_front();
}

template<typename T>
T& Queue<T>::front() {
    return *list.begin();
}

template<typename T>
bool Queue<T>::empty() const {
    return list.empty();
}

template<typename T>
size_t Queue<T>::size() const {
    return list.size();
}
