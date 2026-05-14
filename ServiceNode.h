#ifndef SERVICENODE_H
#define SERVICENODE_H

#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QObject>
#include <QString>
#include <QList>

class EdgeLine;

// =============================================================================
// ServiceNode - draggable circle on the canvas representing one service
//
// Changes from original:
//   - selfWeightLabel: small text inside the node showing the self-resilience
//     weight when a self-edge exists (e.g. "o3" for self-weight 3).
//     Updated by setSelfWeight() called from MainWindow when a self-edge
//     is added via addEdge(id, id, w, false).
//   - mousePressEvent: left click pops up a summary of the service's current
//     failed weight vs total weight so the user can see the load at a glance.
// =============================================================================
class ServiceNode : public QObject, public QGraphicsEllipseItem {
    Q_OBJECT

public:
    static const int DIAMETER = 80;

    ServiceNode(long long serviceId, const QString& name,
                QGraphicsItem* parent = nullptr);

    // setStatus - recolor the node based on simulation result
    // 0=normal(blue), 1=affected(yellow), 2=failed(red)
    void setStatus(int status);

    // setSelfWeight - show self-resilience weight inside the node
    // called when a self-edge is added for this service
    void setSelfWeight(int weight);

    // updateWeightDisplay - refresh the click-summary data
    // called by MainWindow after each simulation step
    void updateWeightDisplay(float failedWeight, int totalWeight);

    void registerEdge(EdgeLine* edge);
    void unregisterEdge(EdgeLine* edge);

    long long getId()   const { return serviceId; }
    QString   getName() const { return nameLabel->toPlainText(); }

signals:
    void requestAddDependent(long long parentId);
    void requestLinkExisting(long long parentId);
    void requestSimulateFailure(long long serviceId);

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
    QVariant itemChange(GraphicsItemChange change,
                        const QVariant& value) override;

private:
    long long           serviceId;
    QGraphicsTextItem*  nameLabel;
    QGraphicsTextItem*  selfWeightLabel;  // shows "o<w>" for self-edges
    QList<EdgeLine*>    edges;

    // cached weight data for the click summary
    float failedWeight;
    int   totalWeight;

    void updateColor(int status);
};

#endif
