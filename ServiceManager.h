#ifndef SERVICEMANAGER_H
#define SERVICEMANAGER_H

#include "Service.h"
#include "Graph.h"
#include "Queue.h"
#include "HashMap.h"
#include "LinkedList.h"
#include <string>

// ServiceManager - central controller for the dependency failure simulator

class ServiceManager {
private:
    // Service storage - heap array that doubles when full
    // Index 0 unused; service IDs start at 1
    Service*  services;
    long long serviceCapacity;  // allocated slots
    long long serviceCount;     // how many services registered so far

    // Core data structures
    Graph         graph;
    StringHashMap nameToId;
    LongHashSet   failedSet;    // services fully failed in sim run

    // Persistent status lists - updated in O(1) on every status change
    IDList normalList;
    IDList affectedList;
    IDList failedList;

    // Global settings
    int failureThreshold;   // 0-100%, service fails when ratio >= this
    int affectMode;         // 0=naive, 1=weighted propagation

    //helper functions
    // growServices - doubles the service array when at capacity
    void growServices();

    // setServiceStatus - updates the service's status field AND moves its ID
    // between the three status lists to keep them consistent
    void setServiceStatus(long long id, int newStatus);

    // propagateWeightToDependent - pushes the delta of new failed weight
    // from parentId to dependentId across the given edge
    void propagateWeightToDependent(long long parentId, long long dependentId, int edgeWeight);

    // evaluateService - recomputes status from current failRatio vs threshold
    // returns the new status so the caller knows whether to enqueue
    int evaluateService(long long id);

public:
    ServiceManager(int failureThreshold = 50, int affectMode = 1);
    ~ServiceManager();

    // addService - register a new service, auto-assigns ID, returns it
    long long addService(const std::string& name);

    // addEdge - add a dependency edge, returns false if edge already existed
    bool addEdge(long long parentId, long long dependentId,
                 int weight, bool absolute);

    //see phase 1 & 2, this is mainly a wrapper for them
    void simulateFailure(long long failedId);

    // runPhase1 - drains a queue of failed service IDs and propagates failures outward.
    // Newly affected services are added to affectedQueue instead of waiting for a full
    // affectedList rescan.
    void runPhase1(IDQueue& failQueue, IDQueue& affectedQueue);

    // runPhase2 - processes only affected services waiting in affectedQueue.
    // If an affected service causes another service to fail, that failed service is
    // added to failQueue and handled by runPhase1.
    void runPhase2(IDQueue& failQueue, IDQueue& affectedQueue);

    // resetSimulation - restores all services to NORMAL in one O(V) pass
    void resetSimulation();

    // searchByName - look up a service by name, returns its ID or -1
    long long searchByName(const std::string& name) const;

    // Read-only accessors for the Qt UI layer
    bool             isValidId(long long id)      const;
    long long        getServiceCount()             const { return serviceCount; }
    int              getFailureThreshold()         const { return failureThreshold; }
    int              getAffectMode()               const { return affectMode; }
    const Service&   getService(long long id)      const { return services[id]; }
    const EdgeList&  getEdgesFrom(long long id)    const { return graph.getEdgesFrom(id); }
    const IDList&    getNormalList()               const { return normalList; }
    const IDList&    getAffectedList()             const { return affectedList; }
    const IDList&    getFailedList()               const { return failedList; }
};

#endif
