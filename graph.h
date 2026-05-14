#ifndef GRAPH_H
#define GRAPH_H

// =============================================================================
// EdgeNode - one outgoing dependency edge stored in an adjacency list
// =============================================================================
struct EdgeNode {
    long long dependentId;  // the service that depends on the parent
    int       weight;       // importance of this dependency
    bool      absolute;     // if true, dependent immediately fails when parent fails
    EdgeNode* next;

    EdgeNode(long long dep, int w, bool abs)
        : dependentId(dep), weight(w), absolute(abs), next(nullptr) {}
};

// =============================================================================
// EdgeList - singly linked list of EdgeNodes stored per service in the graph
//
// moveFrom() is the key addition over the original:
//   Transfers ownership of this list's heap contents into another EdgeList
//   by swapping raw pointers, then zeroing out the source. This lets
//   Vector::moveResize() relocate EdgeLists without copying heap memory,
//   which would cause double-free crashes since EdgeList has no copy ctor.
// =============================================================================
class EdgeList {
private:
    EdgeNode* head;
    int       listSize;

public:
    EdgeList() : head(nullptr), listSize(0) {}

    ~EdgeList() { clear(); }

    // moveFrom - transfer ownership from other into this, zero out other
    // Called by Vector::moveResize so resize never copies heap memory
    void moveFrom(EdgeList& other) {
        // free anything we currently own
        clear();
        // steal other's contents
        head     = other.head;
        listSize = other.listSize;
        // zero out source so its destructor is a no-op
        other.head     = nullptr;
        other.listSize = 0;
    }

    // push_front - O(1) insert, order in adjacency list doesn't matter
    void push_front(long long dependentId, int weight, bool absolute) {
        EdgeNode* node = new EdgeNode(dependentId, weight, absolute);
        node->next = head;
        head = node;
        listSize++;
    }

    // edgeExists - O(degree) duplicate check before inserting
    // prevents the same edge being added twice from the UI or file
    bool edgeExists(long long dependentId) const {
        EdgeNode* curr = head;
        while (curr) {
            if (curr->dependentId == dependentId) return true;
            curr = curr->next;
        }
        return false;
    }

    EdgeNode* getHead()  const { return head; }
    int       size()     const { return listSize; }
    bool      empty()    const { return head == nullptr; }

    void clear() {
        while (head) {
            EdgeNode* temp = head;
            head = head->next;
            delete temp;
        }
        listSize = 0;
    }
};

// =============================================================================
// Graph - directed weighted adjacency list
//
// Changed from original:
//   - adjList is now a Vector (dynamically resizing) instead of a fixed
//     EdgeList array, so no MAX_SERVICES is needed at construction time.
//   - addEdge now checks edgeExists() before inserting to prevent duplicate
//     edges from being created via the UI.
//   - serviceCount is tracked here so the Vector can grow slot by slot.
//   - getEdgesFrom() added for save functionality and status-list iteration.
// =============================================================================

// forward declare Vector so Graph.h doesn't need to include Vector.h
// (Vector.h includes Graph.h, so including it here would be circular)
class Vector;

class Graph {
private:
    Vector*   adjList;      // dynamically sized, index == service ID
    long long capacity;     // current allocated size (managed by Vector)
    long long serviceCount; // number of services registered so far

public:
    Graph();
    ~Graph();

    // registerService - called when a new service is added
    // grows the adjacency list by one slot and returns the new service's index
    long long registerService();

    // addEdge - adds edge parentId -> dependentId if it doesn't already exist
    // returns false if the edge already existed (caller can warn the user)
    bool addEdge(long long parentId, long long dependentId,
                 int weight, bool absolute);

    // getDependents - mutable ref used by BFS
    EdgeList& getDependents(long long id);

    // getEdgesFrom - const ref used by save and UI read paths
    const EdgeList& getEdgesFrom(long long id) const;

    long long getServiceCount() const { return serviceCount; }
};

#endif
