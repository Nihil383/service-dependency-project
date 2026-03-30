#include "HashMap.h"

HashMap::HashMap() {}

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
}
