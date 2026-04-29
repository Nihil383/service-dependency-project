#include "ServiceManager.h"
#include <iostream>

// =============================================================================
// Constructor / Destructor
// =============================================================================

ServiceManager::ServiceManager(long long max, int threshold, int mode)
    : graph(max), maxServices(max), serviceCount(0),
      failureThreshold(threshold), affectMode(mode) {
    // allocate service array; index 0 is unused (IDs start at 1)
    services = new Service[max + 1];
}

ServiceManager::~ServiceManager() {
    delete[] services;
}

// =============================================================================
// addService
// =============================================================================

long long ServiceManager::addService(const std::string& name) {
    if (serviceCount >= maxServices) {
        std::cout << "[ServiceManager] Cannot add service '" << name
                  << "': at capacity (" << maxServices << ").\n";
        return -1;
    }

    long long id = ++serviceCount;
    services[id] = Service(id, name);

    // register in name→id map for search
    nameToId.put(name, id);

    std::cout << "  Added service '" << name << "' with ID " << id << "\n";
    return id;
}

// =============================================================================
// addEdge
// =============================================================================

void ServiceManager::addEdge(long long parentId, long long dependentId,
                              int weight, bool absolute) {
    if (!isValidId(parentId) || !isValidId(dependentId)) {
        std::cout << "[ServiceManager] addEdge: invalid ID(s) "
                  << parentId << " - " << dependentId << "\n";
        return;
    }

    // The edge goes parent → dependent in the graph
    // (if parent fails, dependent is impacted)
    graph.addEdge(parentId, dependentId, weight, absolute);

    // The dependent's totalWeight grows because it now has one more incoming
    // dependency weight to account for in its fail ratio
    services[dependentId].addIncomingWeight(weight);
}

// =============================================================================
// propagateWeightToDependent — delta weight push
// =============================================================================

void ServiceManager::propagateWeightToDependent(long long parentId,
                                                 long long dependentId,
                                                 int edgeWeight, bool /*absolute*/) {
    Service& parent    = services[parentId];
    Service& dependent = services[dependentId];

    float parentRatio = parent.failRatio();

    // If parent has no dependencies itself (ratio == -1), treat as fully failed
    // (it was explicitly failed via simulateFailure, so ratio should be 1.0
    //  but we guard just in case)
    if (parentRatio < 0.0f) parentRatio = 1.0f;

    float historicRatio = parent.getHistoricRatio();

    if (affectMode == 0) {
        // --- Naive mode ---
        // Push full edge weight regardless of parent's ratio.
        // But still use the delta so we don't double-count if this parent
        // was already partially processed in an earlier simulation step.
        float delta = (parentRatio - historicRatio);
        if (delta <= 0.0f) return; // nothing new to push
        dependent.addFailedWeight((float)edgeWeight * delta);

    } else {
        // --- Weighted mode ---
        // Push (currentRatio - historicRatio) * edgeWeight
        // This means an "affected" parent only partially loads the dependent,
        // proportional to how failed the parent actually is.
        float delta = parentRatio - historicRatio;
        if (delta <= 0.0f) return;
        dependent.addFailedWeight((float)edgeWeight * delta);
    }
}

// =============================================================================
// evaluateService — compare ratio to threshold, set status
// =============================================================================

void ServiceManager::evaluateService(long long id) {
    Service& svc = services[id];

    float ratio = svc.failRatio();

    // Services with no dependencies are immune to propagation
    // (they can only be failed explicitly)
    if (ratio < 0.0f) return;

    int pct = (int)(ratio * 100.0f + 0.5f); // round to nearest percent

    if (pct >= failureThreshold) {
        svc.setStatus(STATUS_FAILED);
    } else if (pct > 0) {
        svc.setStatus(STATUS_AFFECTED);
    }
    // else stays NORMAL
}

// =============================================================================
// simulateFailure — BFS
// =============================================================================

void ServiceManager::simulateFailure(long long failedId) {
    if (!isValidId(failedId)) {
        std::cout << "[simulateFailure] Invalid service ID: " << failedId << "\n";
        return;
    }

    std::cout << "\n=== Simulating failure of service ["
              << failedId << "] " << services[failedId].getName() << " ===\n";

    // --- Step 1: mark the root as failed ---
    services[failedId].setStatus(STATUS_FAILED);
    // A directly failed service has 100% of its own weight failed.
    // We represent this by setting totalFailedWeight = totalWeight.
    // But if it has no dependencies (self-reliant), ratio stays -1 and
    // we still propagate it as a hard failure to dependents.
    if (services[failedId].getTotalWeight() > 0) {
        // Force ratio to 1.0 by adding the full weight
        float needed = (float)services[failedId].getTotalWeight()
                       - services[failedId].getTotalFailedWeight();
        if (needed > 0.0f) services[failedId].addFailedWeight(needed);
    }

    // Add to failed set so we won't re-process it
    failedSet.insert(failedId);

    IDQueue queue;
    queue.enqueue(failedId);

    while (!queue.empty()) {
        long long currentId = queue.front();
        queue.dequeue();

        // Walk every dependent of currentId
        EdgeList& edges = graph.getDependents(currentId);
        EdgeNode* edge  = edges.getHead();

        while (edge != nullptr) {
            long long depId = edge->dependentId;

            // --- a. Already fully failed and processed → skip ---
            if (failedSet.contains(depId)) {
                edge = edge->next;
                continue;
            }

            // --- b. Absolute edge → immediate failure, no ratio math ---
            if (edge->absolute) {
                Service& dep = services[depId];
                dep.setStatus(STATUS_FAILED);

                // force weight to full so ratio is 1.0 for downstream
                float needed = (float)dep.getTotalWeight() - dep.getTotalFailedWeight();
                if (needed > 0.0f) dep.addFailedWeight(needed);

                failedSet.insert(depId); //insert to hash and skip
                queue.enqueue(depId);

                std::cout << "  [ABSOLUTE] " << dep.getName()
                          << " (ID " << depId << ")  FAILED\n";

            } else {
                // --- c. Weight-based propagation ---
                propagateWeightToDependent(currentId, depId,
                                           edge->weight, edge->absolute);
                evaluateService(depId);

                Service& dep = services[depId];

                if (dep.getStatus() == STATUS_FAILED) {
                    // force weight full so it propagates at 1.0 downstream
                    float needed = (float)dep.getTotalWeight() - dep.getTotalFailedWeight();
                    if (needed > 0.0f) dep.addFailedWeight(needed);

                    failedSet.insert(depId); //insert to hash and skip
                    queue.enqueue(depId);

                    std::cout << "  " << dep.getName()
                              << " (ID " << depId << ")  FAILED ("
                              << (int)(dep.failRatio()*100+0.5f) << "% >= "
                              << failureThreshold << "% threshold)\n";

                } else if (dep.getStatus() == STATUS_AFFECTED) {
                    // Enqueue affected services too so they can propagate
                    // their partial weight to their own dependents
                    queue.enqueue(depId);

                    std::cout << "  " << dep.getName()
                              << " (ID " << depId << ")  AFFECTED ("
                              << (int)(dep.failRatio()*100+0.5f) << "% < "
                              << failureThreshold << "% threshold)\n";
                }
            }

            edge = edge->next;
        }

        // --- Step 5: after processing all of currentId's dependents,
        //     update its historicRatio so future propagations from this
        //     node only push the delta ---
        services[currentId].updateHistoricRatio();
    }

    std::cout << "=== Simulation complete ===\n\n";
}

// =============================================================================
// resetSimulation
// =============================================================================

void ServiceManager::resetSimulation() {
    for (long long i = 1; i <= serviceCount; i++) {
        services[i].resetForSimulation();
    }
    failedSet.clear();
    std::cout << "[Reset] All services restored to NORMAL.\n";
}

// =============================================================================
// searchByName
// =============================================================================

void ServiceManager::searchByName(const std::string& name) const {
    long long id;
    if (!nameToId.get(name, id)) {
        std::cout << "No service found with name '" << name << "'.\n";
        return;
    }
    services[id].printStatus();
}

// =============================================================================
// printAll
// =============================================================================

void ServiceManager::printAll() const {
    std::cout << "\n--- All Services ---\n";
    for (long long i = 1; i <= serviceCount; i++) {
        services[i].printStatus();
    }
    std::cout << "--------------------\n";
}

// =============================================================================
// isValidId
// =============================================================================

bool ServiceManager::isValidId(long long id) const {
    return (id >= 1 && id <= serviceCount);
}
