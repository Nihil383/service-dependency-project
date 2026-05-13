#ifndef HASHMAP_H
#define HASHMAP_H

#include <string>
#include "LinkedList.h"

using namespace std;

// HashMap Data Structure
// Used for tracking service status in Service Dependency Failure Simulator

/*class HashMap {
private:
    struct Pair {
        string key;
        string value;

        Pair(const string& k, const string& v)
            : key(k), value(v) {}
    };

    static const int TABLE_SIZE = 10;
    LinkedList<Pair> table[TABLE_SIZE];

    int hashFunction(const string& key) const;

public:
    HashMap();

    void put(const string& key, const string& value);

    bool get(const string& key, string& value) const;

    bool contains(const string& key) const;

    bool remove(const string& key);
};*/

// LongHashSet — a set of long long service IDs
// Used for: BFS failed-set — O(1) average insert and lookup so we can instantly
// skip any service that has already been fully processed as failed,
// avoiding re-walking its dependents over and over
// Collision resolution: linked list
// Table size 101

class LongHashSet {
private:
    static const int TABLE_SIZE = 101;
    LongSetNode* table[TABLE_SIZE];

    //multiplicative hash function
    int hashFunction(long long key) const;

public:
    LongHashSet();
    ~LongHashSet();
    void insert(long long key);
    bool contains(long long key) const;
    void clear();
};

//StringHashMap — maps string keys to long long values
//Used for: service name to service ID lookup
//Collision resolution: linked list

class StringHashMap {
private:
    static const int TABLE_SIZE = 101;
    StringMapNode* table[TABLE_SIZE];

    //multiplicative hash function
    int hashFunction(const string& key) const;

public:
    StringHashMap();
    ~StringHashMap();
    void put(const string& key, long long value);
    bool get(const string& key, long long& outValue) const;
    bool contains(const string& key) const;
    bool remove(const string& key);
};

#endif
