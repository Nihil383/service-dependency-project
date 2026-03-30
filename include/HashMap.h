#ifndef HASHMAP_H
#define HASHMAP_H

#include <string>
#include "LinkedList.h"

using namespace std;

// HashMap Data Structure
// Used for tracking service status in Service Dependency Failure Simulator

class HashMap {
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
};

#include "HashMap.cpp"

#endif
