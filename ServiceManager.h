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
    long long serviceCapacity;
    long long serviceCount;

    // Core data structures
    Graph         graph;
    StringHashMap nameToId;
    LongHashSet   failedSet;

    // Persistent status lists
    IDList normalList;
    IDList affectedList;
    IDList failedList;

    // Global settings
    int failureThreshold;
    int affectMode;

    // Helper functions
    void growServices();

    void setServiceStatus(long long id, int newStatus);

    void propagateWeightToDependent(long long parentId,
                                    long long dependentId,
                                    int edgeWeight);

    int evaluateService(long long id);

public:
    ServiceManager(int failureThreshold = 50, int affectMode = 1);
    ~ServiceManager();

    long long addService(const std::string& name);

    bool addEdge(long long parentId,
                 long long dependentId,
                 int weight,
                 bool absolute);

    void simulateFailure(long long failedId);

    void runPhase1(IDQueue& failQueue, IDQueue& affectedQueue);

    void runPhase2(IDQueue& failQueue, IDQueue& affectedQueue);

    void resetSimulation();

    long long searchByName(const std::string& name) const;

    // Read-only accessors for the Qt UI layer
    bool             isValidId(long long id)      const;
    long long        getServiceCount()            const { return serviceCount; }
    int              getFailureThreshold()        const { return failureThreshold; }
    int              getAffectMode()              const { return affectMode; }
    const Service&   getService(long long id)     const { return services[id]; }
    const EdgeList&  getEdgesFrom(long long id)   const { return graph.getEdgesFrom(id); }
    const IDList&    getNormalList()              const { return normalList; }
    const IDList&    getAffectedList()            const { return affectedList; }
    const IDList&    getFailedList()              const { return failedList; }
};

#endif
