#ifndef SERVICEMANAGER_H
#define SERVICEMANAGER_H

#include "Service.h"
#include "Graph.h"
#include "Queue.h"
#include "HashMap.h"
#include <string>

// =============================================================================
// ServiceManager — central controller for the dependency failure simulator
//
// Owns:
//   services[]     — array of Service objects, indexed by ID (1..maxServices)
//   graph          — adjacency list of directed weighted edges
//   nameToId       — StringHashMap for name→ID search
//   failedSet      — LongHashMap used during BFS to track already-fully-failed
//                    services so we don't re-process their dependents
//
// Global settings:
//   failureThreshold — percentage (0–100) at which a service is considered
//                      FAILED rather than just AFFECTED (default 50)
//   affectMode       — controls how affected services propagate weight:
//                        0 = naive (affected parent contributes full edge weight)
//                        1 = weighted (affected parent contributes ratio * weight)
// =============================================================================
class ServiceManager {
private:
    Service*      services;         // services[id] = Service with that id
    Graph         graph;
    StringHashMap nameToId;         // name → id (for search by name)
    LongHashSet   failedSet;        // set of IDs fully failed THIS sim run
    long long     maxServices;      // capacity
    long long     serviceCount;     // how many services have been added so far
    int           failureThreshold; // 0–100, default 50
    int           affectMode;       // 0=naive, 1=weighted propagation

    // -------------------------------------------------------------------------
    // propagateWeightToDependent — core weight calculation called during BFS
    //
    // When a parent (parentId) has just been processed, we push the appropriate
    // amount of failed weight into the dependent across the given edge.
    //
    // The delta approach:
    //   Last time this parent propagated (its historicRatio), it may have already
    //   pushed some weight to this dependent. Now its ratio may be higher.
    //   We only push the NEW portion: (currentRatio - historicRatio) * edgeWeight
    //   This prevents double-counting across multiple simulation events.
    //
    // affectMode==0 (naive): if parent is failed (ratio==1.0), push full weight.
    //                        if parent is only affected, push full weight anyway.
    // affectMode==1 (weighted): push ratio * weight, using the delta trick above.
    // -------------------------------------------------------------------------
    void propagateWeightToDependent(long long parentId, long long dependentId,
                                    int edgeWeight, bool absolute);

    // -------------------------------------------------------------------------
    // evaluateService — after weight is updated on a service, check its ratio
    // against failureThreshold and set STATUS_FAILED or STATUS_AFFECTED
    // -------------------------------------------------------------------------
    void evaluateService(long long id);

public:
    // -------------------------------------------------------------------------
    // Constructor — called after input file is parsed so we know maxServices,
    // failureThreshold, and affectMode upfront
    // -------------------------------------------------------------------------
    ServiceManager(long long maxServices, int failureThreshold, int affectMode);

    ~ServiceManager();

    // -------------------------------------------------------------------------
    // addService — register a new service by name, auto-assigns next ID
    // Returns the assigned ID, or -1 if at capacity
    // -------------------------------------------------------------------------
    long long addService(const std::string& name);

    // -------------------------------------------------------------------------
    // addEdge — add a dependency edge: parentId depends-on-by dependentId
    //   (meaning if parent fails, dependent is affected)
    //   Also increments dependent's totalWeight so ratios stay correct
    // -------------------------------------------------------------------------
    void addEdge(long long parentId, long long dependentId, int weight, bool absolute);

    // -------------------------------------------------------------------------
    // simulateFailure — BFS failure propagation starting from failedId
    //
    // Algorithm per your spec:
    //  1. Mark failedId as STATUS_FAILED, add to failedSet, enqueue
    //  2. Dequeue current service
    //  3. Walk its EdgeList of dependents
    //  4. For each dependent:
    //     a. If dependent already in failedSet → skip entirely
    //     b. If edge is absolute → mark dependent FAILED, add to failedSet, enqueue
    //     c. Otherwise → propagateWeightToDependent, then evaluateService
    //        If evaluation produced FAILED → add to failedSet, enqueue
    //        If AFFECTED → enqueue so its own dependents get updated too
    //  5. After processing all dependents of current node → updateHistoricRatio
    //  6. Continue until queue empty
    // -------------------------------------------------------------------------
    void simulateFailure(long long failedId);

    // -------------------------------------------------------------------------
    // resetSimulation — restore all services to NORMAL, clear failedSet
    // Called before each fresh simulation run
    // -------------------------------------------------------------------------
    void resetSimulation();

    // -------------------------------------------------------------------------
    // searchByName — look up a service by name, print its status
    // -------------------------------------------------------------------------
    void searchByName(const std::string& name) const;

    // -------------------------------------------------------------------------
    // printAll — dump all registered services and their current status
    // -------------------------------------------------------------------------
    void printAll() const;

    // -------------------------------------------------------------------------
    // isValidId — bounds + registration check
    // -------------------------------------------------------------------------
    bool isValidId(long long id) const;

    long long getServiceCount()  const { return serviceCount; }
    int getFailureThreshold()    const { return failureThreshold; }
    int getAffectMode()          const { return affectMode; }
    long long getCapacity()      const { return maxServices; }

    // -------------------------------------------------------------------------
    // getService — read-only access to a Service by ID for the Qt UI layer
    // Qt uses this to read name and status for rendering nodes on the canvas
    // Never call with an invalid ID — check isValidId first
    // -------------------------------------------------------------------------
    const Service& getService(long long id) const { return services[id]; }
};

#endif
