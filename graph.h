#ifndef GRAPH_H
#define GRAPH_H

// =============================================================================
// EdgeList — Singly linked list of directed edges
// Each node in this list represents one outgoing dependency edge:
//   parentId --[weight, absolute]--> dependentId
// This list is stored per-service inside the Graph array.
// =============================================================================

struct EdgeNode {
    long long dependentId;  // The service that depends on the parent
    int weight;             // How important this dependency is (used in ratio calc)
    bool absolute;          // If true, dependent immediately fails when parent fails
    EdgeNode* next;

    EdgeNode(long long dep, int w, bool abs)
        : dependentId(dep), weight(w), absolute(abs), next(nullptr) {
    }
};

class EdgeList {
private:
    EdgeNode* head;
    int listSize;

public:
    EdgeList() : head(nullptr), listSize(0) {}

    ~EdgeList() {
        clear();
    }

    //insert
    void push_front(long long dependentId, int weight, bool absolute) {
        EdgeNode* node = new EdgeNode(dependentId, weight, absolute);
        node->next = head;
        head = node;
        listSize++;
    }

    // getHead — lets Graph and ServiceManager traverse the edge list
    EdgeNode* getHead() const {
        return head;
    }

    int size() const {
        return listSize;
    }

    bool empty() const {
        return head == nullptr;
    }

    // clear — free all nodes
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
// Graph — Directed weighted adjacency list
// Array index == service ID, so lookup is O(1)
// Each slot holds an EdgeList of outgoing edges (parent → dependents)
// Max capacity is fixed at construction time from the input file
// =============================================================================

class Graph {
private:
    EdgeList* adjList;  // adjList[id] = list of all services that depend on id
    long long capacity; // max number of services (set at program start)

public:
    // -------------------------------------------------------------------------
    // Constructor — allocates array of EdgeLists sized to max+1
    // (index 0 unused; IDs start at 1)
    // -------------------------------------------------------------------------
    Graph(long long maxServices);

    ~Graph();

    // -------------------------------------------------------------------------
    // addEdge — adds a directed weighted edge: parentId → dependentId
    //   weight:   importance of this dependency (used in failure ratio calc)
    //   absolute: if true, dependent immediately fails when parent fails
    //             regardless of weight ratio
    // -------------------------------------------------------------------------
    void addEdge(long long parentId, long long dependentId, int weight, bool absolute);

    // -------------------------------------------------------------------------
    // getDependents — returns the EdgeList for a given service ID
    // ServiceManager uses this during BFS to walk all dependents
    // -------------------------------------------------------------------------
    EdgeList& getDependents(long long id);

    long long getCapacity() const { return capacity; }
};

#endif
