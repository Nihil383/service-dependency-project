#ifndef SERVICE_H
#define SERVICE_H

#include <string>
using namespace std;

//Status constants — stored as int on each Service
const int STATUS_NORMAL = 0;
const int STATUS_AFFECTED = 1;
const int STATUS_FAILED = 2;
class Service {
private:
    long long id;
    string name;
    int status;
    int totalWeight; //sum of all incoming edge weights
    float totalFailedWeight; //accumulated failed/affected weight
    float historicRatio; //ratio last propagated outward to our dependents in affected state

public:
    //Constructors
    Service();
    Service(long long id, const string& name);

    //Getters
    long long getId() const { return id; }
    string getName() const { return name; }
    int getStatus() const { return status; }
    int getTotalWeight() const { return totalWeight; }
    float getTotalFailedWeight() const { return totalFailedWeight; }
    float getHistoricRatio() const { return historicRatio; }

    //failRatio — totalFailedWeight / totalWeight
    //Returns -1.0f when totalWeight == 0 (no dependencies)
    float failRatio() const;

    //Setters
    void setStatus(int s) { status = s; }
    void setName(const string& n) { name = n; }

    //addIncomingWeight — called when a new edge pointing TO this service is added, 
    //This keeps totalWeight up to date as the graph is built
    void addIncomingWeight(int w) { totalWeight += w; }

    //addFailedWeight — called during BFS to accumulate failed weight into this node
    void addFailedWeight(float w) { totalFailedWeight += w; }

    //updateHistoricRatio — called after we finish propagating from this node
    //so next time we only push the delta
    void updateHistoricRatio() { historicRatio = failRatio(); }

    // resetForSimulation — clears affected/failed state between simulation runs
    // Does NOT clear totalWeight (graph structure stays intact)
    void resetForSimulation();
    void printStatus() const;
};

#endif