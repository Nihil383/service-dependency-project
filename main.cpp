#include "ServiceManager.h"
#include <iostream>
#include <fstream>
#include <string>

// =============================================================================
// INPUT FILE FORMAT
// =============================================================================
// Line 1:  MAX_SERVICES FAILURE_THRESHOLD AFFECT_MODE
//   MAX_SERVICES     — maximum number of services the program will ever hold
//   FAILURE_THRESHOLD— integer 0–100, percent at which a service is FAILED
//   AFFECT_MODE      — 0 = naive propagation, 1 = weighted propagation
//
// Then any number of command lines in any order:
//
//   SERVICE <name>
//     Register a new service. IDs are auto-assigned starting at 1.
//
//   EDGE <parentId> <dependentId> <weight> <absolute>
//     Add a dependency edge.
//     parentId/dependentId — integer IDs matching the order services were added
//     weight               — positive integer, importance of this dependency
//     absolute             — 0 or 1; if 1, dependent immediately fails when parent fails
//
//   SELF <id> <weight>
//     Add a self-resilience edge: service depends on itself with given weight.
//     This raises totalWeight so that losing other dependencies alone may not
//     be enough to cross the failure threshold.
//
// Blank lines and lines starting with # are ignored (comments).

// Helper: trim leading/trailing spaces from a string
std::string trim(const std::string& s) {
    int start = 0, end = (int)s.size() - 1;
    while (start <= end && (s[start] == ' ' || s[start] == '\t')) start++;
    while (end >= start && (s[end] == ' ' || s[end] == '\t'))   end--;
    return s.substr(start, end - start + 1);
}

// =============================================================================
// loadFromFile — parse the input file and populate the ServiceManager
// Returns nullptr on failure.
// =============================================================================
ServiceManager* loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Error: could not open file '" << filename << "'.\n";
        return nullptr;
    }

    std::string line;
    bool headerRead = false;
    ServiceManager* sm = nullptr;

    while (std::getline(file, line)) {
        line = trim(line);

        // skip blank lines and comments
        if (line.empty() || line[0] == '#') continue;

        if (!headerRead) {
            // --- First real line: global settings ---
            long long maxServices;
            int threshold, mode;
            // manual parse: find space-separated tokens
            // using a simple index scan since no STL stream parsing
            int i = 0, n = (int)line.size();

            // parse maxServices
            maxServices = 0;
            while (i < n && line[i] == ' ') i++;
            while (i < n && line[i] >= '0' && line[i] <= '9')
                maxServices = maxServices * 10 + (line[i++] - '0');

            // parse threshold
            threshold = 0;
            while (i < n && line[i] == ' ') i++;
            while (i < n && line[i] >= '0' && line[i] <= '9')
                threshold = threshold * 10 + (line[i++] - '0');

            // parse mode
            mode = 0;
            while (i < n && line[i] == ' ') i++;
            while (i < n && line[i] >= '0' && line[i] <= '9')
                mode = mode * 10 + (line[i++] - '0');

            std::cout << "Settings: max=" << maxServices
                << " threshold=" << threshold << "%"
                << " mode=" << (mode == 0 ? "naive" : "weighted") << "\n";

            sm = new ServiceManager(maxServices, threshold, mode);
            headerRead = true;
            continue;
        }

        // --- Parse command lines ---
        // Find the command keyword (first token)
        int spacePos = 0;
        while (spacePos < (int)line.size() && line[spacePos] != ' ') spacePos++;
        std::string cmd = line.substr(0, spacePos);
        std::string rest = trim(line.substr(spacePos));

        if (cmd == "SERVICE") {
            // SERVICE <name>  — name may contain spaces
            sm->addService(rest);

        }
        else if (cmd == "EDGE") {
            // EDGE <parentId> <dependentId> <weight> <absolute>
            long long parentId = 0, dependentId = 0;
            int weight = 0, absolute = 0;
            int i = 0, n = (int)rest.size();

            while (i < n && rest[i] == ' ') i++;
            while (i < n && rest[i] >= '0' && rest[i] <= '9')
                parentId = parentId * 10 + (rest[i++] - '0');

            while (i < n && rest[i] == ' ') i++;
            while (i < n && rest[i] >= '0' && rest[i] <= '9')
                dependentId = dependentId * 10 + (rest[i++] - '0');

            while (i < n && rest[i] == ' ') i++;
            while (i < n && rest[i] >= '0' && rest[i] <= '9')
                weight = weight * 10 + (rest[i++] - '0');

            while (i < n && rest[i] == ' ') i++;
            while (i < n && rest[i] >= '0' && rest[i] <= '9')
                absolute = absolute * 10 + (rest[i++] - '0');

            sm->addEdge(parentId, dependentId, weight, absolute != 0);

        }
        else if (cmd == "SELF") {
            // SELF <id> <weight>  — self-resilience edge
            long long id = 0;
            int weight = 0;
            int i = 0, n = (int)rest.size();

            while (i < n && rest[i] == ' ') i++;
            while (i < n && rest[i] >= '0' && rest[i] <= '9')
                id = id * 10 + (rest[i++] - '0');

            while (i < n && rest[i] == ' ') i++;
            while (i < n && rest[i] >= '0' && rest[i] <= '9')
                weight = weight * 10 + (rest[i++] - '0');

            // A self-edge: the service is its own parent
            // addEdge also calls addIncomingWeight on the dependent (itself), 
            // which correctly raises its totalWeight
            sm->addEdge(id, id, weight, false);

        }
        else {
            std::cout << "[loadFromFile] Unknown command: '" << cmd << "' — skipping.\n";
        }
    }

    file.close();

    if (!sm) {
        std::cout << "Error: file had no valid header line.\n";
    }
    return sm;
}

// =============================================================================
// printMenu
// =============================================================================
void printMenu() {
    std::cout << "\n========== Service Dependency Simulator ==========\n";
    std::cout << "  1. List all services\n";
    std::cout << "  2. Add new service\n";
    std::cout << "  3. Add dependency edge\n";
    std::cout << "  4. Simulate failure by service ID\n";
    std::cout << "  5. Search service by name\n";
    std::cout << "  6. Reset simulation (restore all to NORMAL)\n";
    std::cout << "  0. Exit\n";
    std::cout << "==================================================\n";
    std::cout << "Choice: ";
}

// =============================================================================
// main
// =============================================================================
int main() {
    std::string filename;
    std::cout << "Enter input file path: ";
    std::cin >> filename;
    std::cin.ignore(); // clear newline after filename

    ServiceManager* sm = loadFromFile(filename);
    if (!sm) return 1;

    int choice;
    do {
        printMenu();
        std::cin >> choice;
        std::cin.ignore();

        if (choice == 1) {
            // --- List all services ---
            sm->printAll();

        }
        else if (choice == 2) {
            // --- Add new service ---
            std::cout << "Service name: ";
            std::string name;
            std::getline(std::cin, name);
            long long id = sm->addService(name);
            if (id != -1)
                std::cout << "  Service added with ID " << id << "\n";

        }
        else if (choice == 3) {
            // --- Add dependency edge ---
            long long parentId, dependentId;
            int weight, absInt;

            std::cout << "Parent ID (if parent fails, dependent is impacted): ";
            std::cin >> parentId;
            std::cout << "Dependent ID: ";
            std::cin >> dependentId;
            std::cout << "Edge weight (positive int): ";
            std::cin >> weight;
            std::cout << "Absolute? (1=yes, dependent immediately fails; 0=no): ";
            std::cin >> absInt;
            std::cin.ignore();

            sm->addEdge(parentId, dependentId, weight, absInt != 0);
            std::cout << "  Edge added.\n";

        }
        else if (choice == 4) {
            // --- Simulate failure ---
            long long id;
            std::cout << "Service ID to fail: ";
            std::cin >> id;
            std::cin.ignore();
            sm->simulateFailure(id);

        }
        else if (choice == 5) {
            // --- Search by name ---
            std::cout << "Service name: ";
            std::string name;
            std::getline(std::cin, name);
            sm->searchByName(name);

        }
        else if (choice == 6) {
            // --- Reset ---
            sm->resetSimulation();

        }
        else if (choice != 0) {
            std::cout << "Invalid choice.\n";
        }

    } while (choice != 0);

    delete sm;
    std::cout << "Goodbye.\n";
    return 0;
}
