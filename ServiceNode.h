#ifndef SERVICENODE_H
#define SERVICENODE_H

#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QObject>
#include <QString>
#include <QList>

class EdgeLine;  // forward declare to avoid circular include

// =============================================================================
// ServiceNode — one draggable circle on the canvas representing a service
//
// Inherits QObject (for signals) and QGraphicsEllipseItem (for the scene).
// Stores the service's ID so MainWindow can call back into ServiceManager.
//
// Status colors:
//   Normal   → blue   (#4A90D9)
//   Affected → yellow (#F5A623)
//   Failed   → red    (#D0021B)
// =============================================================================
class ServiceNode : public QObject, public QGraphicsEllipseItem {
    Q_OBJECT

public:
    // node diameter in pixels
    static const int DIAMETER = 80;

    ServiceNode(long long serviceId, const QString& name, QGraphicsItem* parent = nullptr);

    // -----------------------------------------------------------------------
    // setStatus — called after a simulation run to recolor the node
    //   0 = normal, 1 = affected, 2 = failed
    // -----------------------------------------------------------------------
    void setStatus(int status);

    // -----------------------------------------------------------------------
    // registerEdge / unregisterEdge — EdgeLine calls these so the node
    // knows which lines to reposition when it is dragged
    // -----------------------------------------------------------------------
    void registerEdge(EdgeLine* edge);
    void unregisterEdge(EdgeLine* edge);

    long long getServiceId() const { return serviceId; }
    QString   getName()      const { return nameLabel->toPlainText(); }

signals:
    // emitted from the right click menu — MainWindow connects to these
    void requestAddDependent(long long parentId);
    void requestLinkExisting(long long parentId);
    void requestSimulateFailure(long long serviceId);

protected:
    // right click menu
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

    // reposition all attached edges whenever this node moves
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    long long           serviceId;
    QGraphicsTextItem*  nameLabel;
    QList<EdgeLine*>    edges;      // all edges touching this node

    void updateColor(int status);
};

#endif
