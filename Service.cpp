#include "Service.h"
#include <iostream>
#include <string>
using namespace std;

Service::Service()
    : id(0), name("UNSET"), status(STATUS_NORMAL),
    totalWeight(0), totalFailedWeight(0.0f), historicRatio(0.0f) {
}

Service::Service(long long id, const string& name)
    : id(id), name(name), status(STATUS_NORMAL),
    totalWeight(0), totalFailedWeight(0.0f), historicRatio(0.0f) {
}

float Service::failRatio() const {
    // No dependencies at all - ratio holds sentinel value
    if (totalWeight == 0) return -1.0f;
    return totalFailedWeight / (float)totalWeight;
}

void Service::resetForSimulation() {
    // Only reset dynamic simulation state.
    // totalWeight stays because the graph structure hasn't changed.
    status = STATUS_NORMAL;
    totalFailedWeight = 0.0f;
    historicRatio = 0.0f;
}

void Service::printStatus() const {
    string statusStr;
    if (status == STATUS_NORMAL)   statusStr = "NORMAL";
    else if (status == STATUS_AFFECTED) statusStr = "AFFECTED";
    else                                statusStr = "FAILED";

    cout << "[ID: " << id << "] " << name << " - " << statusStr;

    if (totalWeight > 0) {
        // show fail ratio as a percentage rounded to nearest int
        int pct = (int)(failRatio() * 100.0f + 0.5f);
        cout << " (" << pct << "% failure load)";
    }
    else {
        cout << " (no dependencies)";
    }
    cout << "\n";
}