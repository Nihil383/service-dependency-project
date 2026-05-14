#include "ServiceManager.h"
#include <iostream>

ServiceManager::ServiceManager(int threshold, int mode)
    : serviceCapacity(16), serviceCount(0),
      failureThreshold(threshold), affectMode(mode)
{
    // start with 15 slots (index 0 unused)
    services = new Service[serviceCapacity];
}

ServiceManager::~ServiceManager() {
    delete[] services;
}

// growServices - vector doubling operation
void ServiceManager::growServices() {
    long long newCap = serviceCapacity * 2;
    Service* newArr  = new Service[newCap];
    for (long long i = 0; i <= serviceCount; i++) {
        newArr[i] = services[i];
    }
    delete[] services;
    services         = newArr;
    serviceCapacity  = newCap;
}

// setServiceStatus
// Moves the service ID between the three status lists AND updates the field.
// All status changes everywhere must go through this so the lists stay correct.
void ServiceManager::setServiceStatus(long long id, int newStatus) {
    int oldStatus = services[id].getStatus();
    if (oldStatus == newStatus) return;

    // remove from current list
    if      (oldStatus == STATUS_NORMAL)   normalList.remove(id);
    else if (oldStatus == STATUS_AFFECTED) affectedList.remove(id);
    else                                   failedList.remove(id);

    // insert into new list
    if      (newStatus == STATUS_NORMAL)   normalList.pushFront(id);
    else if (newStatus == STATUS_AFFECTED) affectedList.pushFront(id);
    else                                   failedList.pushFront(id);

    services[id].setStatus(newStatus);
}

//addService
long long ServiceManager::addService(const std::string& name) {
    // grow if needed (serviceCount+1 because index 0 is unused)
    if (serviceCount + 1 >= serviceCapacity) {
        growServices();
    }

    long long id = ++serviceCount;

    // register a slot in the graph's adjacency list
    graph.registerService();

    // construct the service object
    services[id] = Service(id, name);

    // all new services start NORMAL
    normalList.pushFront(id);

    // register in name->id map for search
    nameToId.put(name, id);

    std::cout << "  Added service '" << name << "' with ID " << id << "\n";
    return id;
}

//addEdge
bool ServiceManager::addEdge(long long parentId, long long dependentId, int weight, bool absolute) {
    if (!isValidId(parentId) || !isValidId(dependentId)) {
        std::cout << "[ServiceManager] addEdge: invalid ID(s) "
                  << parentId << " -> " << dependentId << "\n";
        return false;
    }

    // graph.addEdge does the duplicate check and returns false if it exists
    if (!graph.addEdge(parentId, dependentId, weight, absolute)) {
        return false;
    }

    services[dependentId].addIncomingWeight(weight);
    return true;
}


// propagateWeightToDependent - delta pushes since last processed
// Only pushes the NEW portion of weight since last time this parent
// propagated, so re-processing an affected parent doesn't double-count.
void ServiceManager::propagateWeightToDependent(long long parentId, long long dependentId, int edgeWeight) {
    Service& parent    = services[parentId];
    Service& dependent = services[dependentId];

    float parentRatio = parent.failRatio();
    // if parent has no dependencies of its own, treat it as fully failed
    if (parentRatio < 0.0f) parentRatio = 1.0f;

    float delta = parentRatio - parent.getHistoricRatio();
    if (delta <= 0.0f) return; // nothing new to push

    if (affectMode == 0) {
        // naive: push full edge weight regardless of parent's partial ratio
        dependent.addFailedWeight((float)edgeWeight);
    } else {
        // weighted: push proportionally to how failed the parent actually is
        dependent.addFailedWeight((float)edgeWeight * delta);
    }
}

// evaluateService - check ratio against threshold, return new status
int ServiceManager::evaluateService(long long id) {
    float ratio = services[id].failRatio();
    if (ratio < 0.0f) return STATUS_NORMAL; // no dependencies, immune

    int pct = (int)(ratio * 100.0f + 0.5f);
    if (pct >= failureThreshold) return STATUS_FAILED;
    if (pct > 0)                 return STATUS_AFFECTED;
    return STATUS_NORMAL;
}

/*runPhase1 - failure-only BFS
Drains failQueue, walking outgoing edges of each failed node.
Absolute edges only fire if the current node is fully FAILED.
Affected nodes accumulate weight but stay out of the queue —
phase 2 handles them after this settles.*/
void ServiceManager::runPhase1(IDQueue& failQueue) {
    while (!failQueue.empty()) {
        long long currentId = failQueue.front();
        failQueue.dequeue();

        EdgeList& edges = graph.getDependents(currentId);
        EdgeNode* edge  = edges.getHead();

        while (edge) {
            long long depId = edge->dependentId;

            // already fully processed as a failure source - skip
            if (failedSet.contains(depId)) {
                edge = edge->next;
                continue;
            }

            if (edge->absolute) {
                // only fire if parent is fully failed, not merely affected
                if (services[currentId].getStatus() == STATUS_FAILED) {
                    Service& dep = services[depId];
                    setServiceStatus(depId, STATUS_FAILED);
                    float needed = (float)dep.getTotalWeight()
                                   - dep.getTotalFailedWeight();
                    if (needed > 0.0f) dep.addFailedWeight(needed);
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
                    float needed = (float)dep.getTotalWeight()
                                   - dep.getTotalFailedWeight();
                    if (needed > 0.0f) dep.addFailedWeight(needed);
                    failedSet.insert(depId);
                    failQueue.enqueue(depId);
                    std::cout << "  " << dep.getName()
                              << " (ID " << depId << ") -> FAILED ("
                              << (int)(dep.failRatio()*100+0.5f)
                              << "% >= " << failureThreshold << "%)\n";

                } else if (newStatus == STATUS_AFFECTED) {
                    setServiceStatus(depId, STATUS_AFFECTED);
                    std::cout << "  " << services[depId].getName()
                              << " (ID " << depId << ") -> AFFECTED ("
                              << (int)(services[depId].failRatio()*100+0.5f)
                              << "% < " << failureThreshold << "%)\n";
                }
            }

            edge = edge->next;
        }

        services[currentId].updateHistoricRatio();
    }
}

/*
runPhase2 - affected node propagation with cascade handling
Takes a snapshot of the current affectedList so we iterate a fixed set.
For each affected node, pushes partial weight to its dependents.
If a dependent tips over the threshold:
- marks it failed, runs phase1 on it (which may produce new affected nodes)
- sets newFailuresFound = true so we loop again with a fresh snapshot
Keeps looping until a full pass produces zero new failures.
Convergence guaranteed: each iteration strictly increases the failed count,
which is bounded by V. So at most V iterations total.
*/
void ServiceManager::runPhase2() {
    bool newFailuresFound = true;

    while (newFailuresFound) {
        newFailuresFound = false;

        std::cout << "  [Phase 2] Propagating affected node weights...\n";

        // snapshot the current affected list so new entries added during
        // this pass don't get processed mid-iteration
        IDQueue snapshot;
        IDListNode* curr = affectedList.getHead();
        while (curr) {
            snapshot.enqueue(curr->id);
            curr = curr->next;
        }

        while (!snapshot.empty()) {
            long long affId = snapshot.front();
            snapshot.dequeue();

            // may have been failed by a cascade earlier in this same pass
            if (services[affId].getStatus() != STATUS_AFFECTED) continue;

            EdgeList& edges = graph.getDependents(affId);
            EdgeNode* edge  = edges.getHead();

            while (edge) {
                long long depId = edge->dependentId;

                if (failedSet.contains(depId)) {
                    edge = edge->next;
                    continue;
                }

                // absolute edges from affected parents never fire here
                if (!edge->absolute) {
                    propagateWeightToDependent(affId, depId, edge->weight);
                    int newStatus = evaluateService(depId);

                    if (newStatus == STATUS_FAILED) {
                        Service& dep = services[depId];
                        setServiceStatus(depId, STATUS_FAILED);
                        float needed = (float)dep.getTotalWeight()
                                       - dep.getTotalFailedWeight();
                        if (needed > 0.0f) dep.addFailedWeight(needed);
                        failedSet.insert(depId);

                        // recursively run phase 1 for this new failure -
                        // it may produce more affected nodes which will be
                        // caught in the next outer loop iteration
                        IDQueue newFailQueue;
                        newFailQueue.enqueue(depId);
                        runPhase1(newFailQueue);

                        newFailuresFound = true; // trigger another phase 2 pass
                        std::cout << "  [P2] " << dep.getName()
                                  << " (ID " << depId << ") -> FAILED\n";

                    } else if (newStatus == STATUS_AFFECTED &&
                               services[depId].getStatus() != STATUS_AFFECTED) {
                        setServiceStatus(depId, STATUS_AFFECTED);
                        std::cout << "  [P2] " << services[depId].getName()
                                  << " (ID " << depId << ") -> AFFECTED\n";
                    }
                }

                edge = edge->next;
            }

            services[affId].updateHistoricRatio();
        }
    }
}

// simulateFailure - entry point
// Sets up the root failure and hands off to the two phases.
void ServiceManager::simulateFailure(long long failedId) {
    if (!isValidId(failedId)) {
        std::cout << "[simulateFailure] Invalid service ID: " << failedId << "\n";
        return;
    }

    std::cout << "\n=== Simulating failure of ["
              << failedId << "] " << services[failedId].getName() << " ===\n";

    // force root to fully failed
    setServiceStatus(failedId, STATUS_FAILED);
    if (services[failedId].getTotalWeight() > 0) {
        float needed = (float)services[failedId].getTotalWeight()
        - services[failedId].getTotalFailedWeight();
        if (needed > 0.0f) services[failedId].addFailedWeight(needed);
    }
    failedSet.insert(failedId);

    // phase 1 - propagate the initial failure wave
    IDQueue failQueue;
    failQueue.enqueue(failedId);
    runPhase1(failQueue);

    // phase 2 - propagate affected nodes, looping until stable
    runPhase2();

    std::cout << "=== Simulation complete ===\n\n";
}

// resetSimulation - O(V) pass restores everything and rebuilds status lists
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

// searchByName
long long ServiceManager::searchByName(const std::string& name) const {
    long long id;
    if (!nameToId.get(name, id)) return -1;
    return id;
}

// isValidId
bool ServiceManager::isValidId(long long id) const {
    return (id >= 1 && id <= serviceCount);
}
