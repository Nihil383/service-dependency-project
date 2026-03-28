#include "LinkedList.h"
#include <iostream>
#include <string>

int main() {
    LinkedList<std::string> services;
    
    services.push_back("Database");
    services.push_back("API");
    services.push_front("Auth");
    
    std::cout << "Services: ";
    services.print();
    std::cout << "\nSize: " << services.size() << std::endl;
    
    services.remove("API");
    std::cout << "After removal: ";
    services.print();
    std::cout << std::endl;
    
    return 0;
}