#ifndef VECTOR_H
#define VECTOR_H

#include "Graph.h"

// =============================================================================
// Vector - dynamically resizing array of EdgeList objects
//
// Replaces the fixed-size EdgeList array inside Graph so that the graph
// grows automatically as services are added, removing the need for a
// MAX_SERVICES declaration at startup.
//
// Key modification from the original Vector.cpp:
//   The original resize() copied EdgeList objects by value, which causes
//   double-free crashes because EdgeList owns heap memory (its EdgeNode
//   chain) and has no copy constructor - two objects would point to the
//   same nodes and both destructors would free them.
//
//   Fix: resize() now uses a move-swap approach. It swaps the internal
//   head pointer and size out of each old slot into the new slot, then
//   zeros out the old slot so its destructor is a no-op. This transfers
//   ownership without copying any heap memory.
// =============================================================================
class Vector {
private:
    EdgeList*   data;       // heap array of EdgeList objects
    long long   cap;        // allocated slots
    long long   sz;         // used slots (== serviceCount + 1, index 0 unused)

    // moveResize - allocates a larger array and moves EdgeLists into it
    // without copying their heap contents
    void moveResize(long long newCap) {
        if (newCap <= cap) return;

        EdgeList* newData = new EdgeList[newCap];

        // transfer each EdgeList by swapping its internals
        // EdgeList::moveFrom zeroes out the source so its destructor
        // won't double-free the nodes we just transferred
        for (long long i = 0; i < sz; i++) {
            newData[i].moveFrom(data[i]);
        }

        delete[] data;  // remaining slots are empty (moved-from), safe to delete
        data = newData;
        cap  = newCap;
    }

public:
    Vector() : data(nullptr), cap(0), sz(0) {}

    ~Vector() {
        delete[] data;
    }

    // ensureCapacity - called by Graph when a new service is added
    // doubles capacity each time to keep amortized O(1) per push
    void ensureCapacity(long long needed) {
        if (needed < cap) return;
        long long newCap = (cap == 0) ? 16 : cap * 2;
        while (newCap <= needed) newCap *= 2;
        moveResize(newCap);
    }

    // grow - extend the used size by one slot (called when service is added)
    // returns the index of the new slot
    long long grow() {
        ensureCapacity(sz + 1);
        return sz++;  // caller uses this index
    }

    long long size()     const { return sz; }
    long long capacity() const { return cap; }

    EdgeList& operator[](long long index) {
        return data[index];
    }

    const EdgeList& operator[](long long index) const {
        return data[index];
    }
};

#endif
