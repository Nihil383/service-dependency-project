#include "HashMap.h"
//hashmap section
/*HashMap::HashMap() {}

int HashMap::hashFunction(const string& key) const {
    int sum = 0;
    for (char c : key) {
        sum += c;
    }
    return sum % TABLE_SIZE;
}

void HashMap::put(const string& key, const string& value) {
    int index = hashFunction(key);

    for (auto& pair : table[index]) {
        if (pair.key == key) {
            pair.value = value;
            return;
        }
    }

    table[index].push_back(Pair(key, value));
}

bool HashMap::get(const string& key, string& value) const {
    int index = hashFunction(key);

    for (auto& pair : table[index]) {
        if (pair.key == key) {
            value = pair.value;
            return true;
        }
    }

    return false;
}

bool HashMap::contains(const string& key) const {
    string temp;
    return get(key, temp);
}

bool HashMap::remove(const string& key) {
    int index = hashFunction(key);

    for (auto& pair : table[index]) {
        if (pair.key == key) {
            return table[index].remove(pair);
        }
    }

    return false;
}*/

//Hashset section
//multiplicative hash function
int LongHashSet::hashFunction(long long key) const {
    unsigned long long k = (unsigned long long)key;
    return (int)((k * 2654435761ULL) % TABLE_SIZE);
}
//constructors and destructors
LongHashSet::LongHashSet() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        table[i] = nullptr;
    }
}

LongHashSet::~LongHashSet() {
    clear();
}

//insert
void LongHashSet::insert(long long key) {
    // don't insert duplicates
    if (contains(key)) return;

    int idx = hashFunction(key);

    //prepend to the chain at this bucket — O(1)
    LongSetNode* node = new LongSetNode(key);
    node->next = table[idx];
    table[idx] = node;
}

//search
bool LongHashSet::contains(long long key) const {
    int idx = hashFunction(key);
    LongSetNode* curr = table[idx];
    while (curr) {
        if (curr->key == key) return true;
        curr = curr->next;
    }
    return false;
}

//clear — wipe all entries, called between simulation runs, or upon destruction
void LongHashSet::clear() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        LongSetNode* curr = table[i];
        while (curr) {
            LongSetNode* temp = curr;
            curr = curr->next;
            delete temp;
        }
        table[i] = nullptr;
    }
}

//StringHash section
//multiplicative hash function
int StringHashMap::hashFunction(const string& key) const {
    unsigned long hash = 0;
    for (int i = 0; i < (int)key.size(); i++) {
        hash = hash * 31 + (unsigned char)key[i];
    }
    return (int)(hash % TABLE_SIZE);
}
//constructors and destructors
StringHashMap::StringHashMap() {
// initialize all buckets to null
    for (int i = 0; i < TABLE_SIZE; i++) {
        table[i] = nullptr;
    }
    }
StringHashMap::~StringHashMap() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        StringMapNode* curr = table[i];
        while (curr) {
            StringMapNode* temp = curr;
            curr = curr->next;
            delete temp;
        }
        table[i] = nullptr;
    }
}

//insertion
void StringHashMap::put(const string& key, long long value) {
    int idx = hashFunction(key);
    StringMapNode* curr = table[idx];

    //check if key already exists to update
    while (curr) {
        if (curr->key == key) {
            curr->value = value;
            return;
        }
        curr = curr->next;
    }

    // new entry — prepend to chain
    StringMapNode* node = new StringMapNode(key, value);
    node->next = table[idx];
    table[idx] = node;
}

//retrieve by key, return false if unavailable
bool StringHashMap::get(const string& key, long long& outValue) const {
    int idx = hashFunction(key);
    StringMapNode* curr = table[idx];
    while (curr) {
        if (curr->key == key) {
            outValue = curr->value;
            return true;
        }
        curr = curr->next;
    }
    return false;
}

//search for key using get and a dummy variable
bool StringHashMap::contains(const string& key) const {
    long long dummy;
    return get(key, dummy);
}

// remove — delete a key from the map (if we figure out node deletion later this'll be used)
bool StringHashMap::remove(const string& key) {
    int idx = hashFunction(key);
    StringMapNode* curr = table[idx];
    StringMapNode* prev = nullptr;

    while (curr) {
        if (curr->key == key) {
            if (prev) prev->next = curr->next;
            else       table[idx] = curr->next;
            delete curr;
            return true;
        }
        prev = curr;
        curr = curr->next;
    }
    return false;
}