#include "ServiceManager.h"
#include <iostream>

ServiceManager::ServiceManager(int threshold, int mode)
    : serviceCapacity(16), serviceCount(0),
      failureThreshold(threshold), affectMode(mode)
{
    services = new Service[serviceCapacity];
}

ServiceManager::~ServiceManager() {
    delete[] services;
}

void ServiceManager::growServices() {
    long long newCap = serviceCapacity * 2;
    Service* newArr  = new Service[newCap];

    for (long long i = 0; i <= serviceCount; i++) {
        newArr[i] = services[i];
    }

    delete[] services;
    services        = newArr;
    serviceCapacity = newCap;
}

void ServiceManager::setServiceStatus(long long id, int newStatus) {
    int oldStatus = services[id].getStatus();
    if (oldStatus == newStatus) return;

    if      (oldStatus == STATUS_NORMAL)   normalList.remove(id);
    else if (oldStatus == STATUS_AFFECTED) affectedList.remove(id);
    else                                   failedList.remove(id);

    if      (newStatus == STATUS_NORMAL)   normalList.pushFront(id);
    else if (newStatus == STATUS_AFFECTED) affectedList.pushFront(id);
    else                                   failedList.pushFront(id);

    services[id].setStatus(newStatus);
}

long long ServiceManager::addService(const std::string& name) {
    if (serviceCount + 1 >= serviceCapacity) {
        growServices();
    }

    long long id = ++serviceCount;

    graph.registerService();
    services[id] = Service(id, name);

    normalList.pushFront(id);
    nameToId.put(name, id);

    std::cout << "  Added service '" << name << "' with ID " << id << "\n";
    return id;
}

bool ServiceManager::addEdge(long long parentId, long long dependentId,
                             int weight, bool absolute) {
    if (!isValidId(parentId) || !isValidId(dependentId)) {
        std::cout << "[ServiceManager] addEdge: invalid ID(s) "
                  << parentId << " -> " << dependentId << "\n";
        return false;
    }

    if (!graph.addEdge(parentId, dependentId, weight, absolute)) {
        return false;
    }

    services[dependentId].addIncomingWeight(weight);
    return true;
}

void ServiceManager::propagateWeightToDependent(long long parentId,
                                                long long dependentId,
                                                int edgeWeight) {
    Service& parent    = services[parentId];
    Service& dependent = services[dependentId];

    float parentRatio = parent.failRatio();

    if (parentRatio < 0.0f) {
        parentRatio = 1.0f;
    }

    float delta = parentRatio - parent.getHistoricRatio();

    if (delta <= 0.0f) {
        return;
    }

    if (affectMode == 0) {
        dependent.addFailedWeight((float)edgeWeight);
    } else {
        dependent.addFailedWeight((float)edgeWeight * delta);
    }
}

int ServiceManager::evaluateService(long long id) {
    float ratio = services[id].failRatio();

    if (ratio < 0.0f) {
        return STATUS_NORMAL;
    }

    int pct = (int)(ratio * 100.0f + 0.5f);

    if (pct >= failureThreshold) {
        return STATUS_FAILED;
    }

    if (pct > 0) {
        return STATUS_AFFECTED;
    }

    return STATUS_NORMAL;
}

void ServiceManager::runPhase1(IDQueue& failQueue, IDQueue& affectedQueue) {
    while (!failQueue.empty()) {
        long long currentId = failQueue.front();
        failQueue.dequeue();

        EdgeList& edges = graph.getDependents(currentId);
        EdgeNode* edge  = edges.getHead();

        while (edge) {
            long long depId = edge->dependentId;

            if (failedSet.contains(depId)) {
                edge = edge->next;
                continue;
            }

            if (edge->absolute) {
                if (services[currentId].getStatus() == STATUS_FAILED) {
                    Service& dep = services[depId];

                    setServiceStatus(depId, STATUS_FAILED);

                    float needed =
                        (float)dep.getTotalWeight() -
                        dep.getTotalFailedWeight();

                    if (needed > 0.0f) {
                        dep.addFailedWeight(needed);
                    }

                    failedSet.insert(depId);
                    failQueue.enqueue(depId);

                    std::cout << "  [ABSOLUTE] " << dep.getName()
                              << " (ID " << depId << ") -> FAILED\n";
                }
            } else {
                propagateWeightToDependent(currentId, depId, edge->weight);

                int newStatus = evaluateService(depId);

                if (newStatus == STATUS_FAILED) {
                    Service& dep = services[depId];

                    setServiceStatus(depId, STATUS_FAILED);

                    float needed =
                        (float)dep.getTotalWeight() -
                        dep.getTotalFailedWeight();

                    if (needed > 0.0f) {
                        dep.addFailedWeight(needed);
                    }

                    failedSet.insert(depId);
                    failQueue.enqueue(depId);

                    std::cout << "  " << dep.getName()
                              << " (ID " << depId << ") -> FAILED ("
                              << (int)(dep.failRatio() * 100 + 0.5f)
                              << "% >= " << failureThreshold << "%)\n";

                } else if (newStatus == STATUS_AFFECTED) {
                    if (services[depId].getStatus() != STATUS_AFFECTED) {
                        setServiceStatus(depId, STATUS_AFFECTED);
                        affectedQueue.enqueue(depId);

                        std::cout << "  " << services[depId].getName()
                                  << " (ID " << depId << ") -> AFFECTED ("
                                  << (int)(services[depId].failRatio() * 100 + 0.5f)
                                  << "% < " << failureThreshold << "%)\n";
                    }
                }
            }

            edge = edge->next;
        }

        services[currentId].updateHistoricRatio();
    }
}

void ServiceManager::runPhase2(IDQueue& failQueue, IDQueue& affectedQueue) {
    while (!affectedQueue.empty() || !failQueue.empty()) {

        if (!failQueue.empty()) {
            runPhase1(failQueue, affectedQueue);
            continue;
        }

        long long affId = affectedQueue.front();
        affectedQueue.dequeue();

        if (services[affId].getStatus() != STATUS_AFFECTED) {
            continue;
        }

        EdgeList& edges = graph.getDependents(affId);
        EdgeNode* edge  = edges.getHead();

        while (edge) {
            long long depId = edge->dependentId;

            if (failedSet.contains(depId)) {
                edge = edge->next;
                continue;
            }

            if (!edge->absolute) {
                int oldStatus = services[depId].getStatus();

                propagateWeightToDependent(affId, depId, edge->weight);

                int newStatus = evaluateService(depId);

                if (newStatus == STATUS_FAILED) {
                    Service& dep = services[depId];

                    setServiceStatus(depId, STATUS_FAILED);

                    float needed =
                        (float)dep.getTotalWeight() -
                        dep.getTotalFailedWeight();

                    if (needed > 0.0f) {
                        dep.addFailedWeight(needed);
                    }

                    failedSet.insert(depId);
                    failQueue.enqueue(depId);

                    std::cout << "  [P2] " << dep.getName()
                              << " (ID " << depId << ") -> FAILED\n";

                } else if (newStatus == STATUS_AFFECTED &&
                           oldStatus != STATUS_AFFECTED) {

                    setServiceStatus(depId, STATUS_AFFECTED);
                    affectedQueue.enqueue(depId);

                    std::cout << "  [P2] "
                              << services[depId].getName()
                              << " (ID " << depId << ") -> AFFECTED\n";
                }
            }

            edge = edge->next;
        }

        services[affId].updateHistoricRatio();
    }
}

void ServiceManager::simulateFailure(long long failedId) {
    if (!isValidId(failedId)) {
        std::cout << "[simulateFailure] Invalid service ID: "
                  << failedId << "\n";
        return;
    }

    std::cout << "\n=== Simulating failure of ["
              << failedId << "] "
              << services[failedId].getName()
              << " ===\n";

    setServiceStatus(failedId, STATUS_FAILED);

    if (services[failedId].getTotalWeight() > 0) {
        float needed =
            (float)services[failedId].getTotalWeight() -
            services[failedId].getTotalFailedWeight();

        if (needed > 0.0f) {
            services[failedId].addFailedWeight(needed);
        }
    }

    failedSet.insert(failedId);

    IDQueue failQueue;
    IDQueue affectedQueue;

    failQueue.enqueue(failedId);

    runPhase1(failQueue, affectedQueue);
    runPhase2(failQueue, affectedQueue);

    std::cout << "=== Simulation complete ===\n\n";
}

void ServiceManager::resetSimulation() {
    normalList.clear();
    affectedList.clear();
    failedList.clear();

    for (long long i = 1; i <= serviceCount; i++) {
        services[i].resetForSimulation();
        normalList.pushFront(i);
    }

    failedSet.clear();

    std::cout << "[Reset] All services restored to NORMAL.\n";
}

long long ServiceManager::searchByName(const std::string& name) const {
    long long id;

    if (!nameToId.get(name, id)) {
        return -1;
    }

    return id;
}

bool ServiceManager::isValidId(long long id) const {
    return (id >= 1 && id <= serviceCount);
}
