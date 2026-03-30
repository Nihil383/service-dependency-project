#include "LinkedList.h"
#include "graph.h"
#include <iostream>
#include <string>
using namespace std;

//global counter
static long long serviceCount = 0;

//function to generate the next unique service id
long long newId() {
    return ++serviceCount;
}

class Service {
private:
    long long id;       //used as the index in graph
    string name;        //normal name
    bool affected;      //will be true when affected

public:
    //Constructors
    Service() : id(newId()), name("NA"), affected(false) {}
    Service(string name) : id(newId()), name(name), affected(false) {}

    //getters
    long long getId() const {return id;}
    string getName() const {return name;}
    bool getAffected() const {return affected;}

    //setters
    void setName(string name) {this->name = name;}
    void setAffected(bool affected) {this->affected = affected;}
};

int main() {
    LinkedList<Service> services;
    
    /*services.push_back(Service("Database"));
    services.push_back(Service("API"));
    services.push_front(Service("Auth"));
    
    std::cout << "Services: ";
    services.print();
    std::cout << "\nSize: " << services.size() << std::endl;
    
    services.remove("API");
    std::cout << "After removal: ";
    services.print();
    std::cout << std::endl;*/
    
    return 0;
}