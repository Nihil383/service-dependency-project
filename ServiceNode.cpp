#include "ServiceNode.h"
#include "EdgeLine.h"
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QBrush>
#include <QPen>
#include <QFont>

ServiceNode::ServiceNode(long long serviceId, const QString& name, QGraphicsItem* parent)
    : QObject(), QGraphicsEllipseItem(0, 0, DIAMETER, DIAMETER, parent),
      serviceId(serviceId)
{
    // default color — normal/blue
    updateColor(0);
    setPen(QPen(Qt::black, 2));

    // center the name label inside the circle
    nameLabel = new QGraphicsTextItem(name, this);
    QFont f = nameLabel->font();
    f.setPointSize(8);
    f.setBold(true);
    nameLabel->setFont(f);

    // reposition label to center after setting text
    QRectF bounds = nameLabel->boundingRect();
    nameLabel->setPos(
        (DIAMETER - bounds.width())  / 2.0,
        (DIAMETER - bounds.height()) / 2.0
    );

    // allow dragging and notify us when position changes so edges follow
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    setZValue(1); // nodes draw on top of edges
}

// -----------------------------------------------------------------------
// setStatus — recolor node based on simulation result
// -----------------------------------------------------------------------
void ServiceNode::setStatus(int status) {
    updateColor(status);
}

void ServiceNode::updateColor(int status) {
    if (status == 0)      setBrush(QBrush(QColor("#4A90D9"))); // normal - blue
    else if (status == 1) setBrush(QBrush(QColor("#F5A623"))); // affected - yellow
    else                  setBrush(QBrush(QColor("#D0021B"))); // failed - red
}

// -----------------------------------------------------------------------
// Edge registration — so edges reposition when this node is dragged
// -----------------------------------------------------------------------
void ServiceNode::registerEdge(EdgeLine* edge) {
    if (!edges.contains(edge))
        edges.append(edge);
}

void ServiceNode::unregisterEdge(EdgeLine* edge) {
    edges.removeAll(edge);
}

// -----------------------------------------------------------------------
// itemChange — called by Qt whenever the item's position changes
// We use it to tell every attached edge to recompute its endpoints
// -----------------------------------------------------------------------
QVariant ServiceNode::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemPositionHasChanged) {
        for (EdgeLine* edge : edges) {
            edge->reposition();
        }
    }
    return QGraphicsEllipseItem::itemChange(change, value);
}

// -----------------------------------------------------------------------
// contextMenuEvent — right click menu on the node
// -----------------------------------------------------------------------
void ServiceNode::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
    QMenu menu;

    QAction* addDep   = menu.addAction("Add new dependent service");
    QAction* linkExst = menu.addAction("Link to existing service");
    menu.addSeparator();
    QAction* simulate = menu.addAction("Simulate failure from here");

    QAction* chosen = menu.exec(event->screenPos());

    if      (chosen == addDep)   emit requestAddDependent(serviceId);
    else if (chosen == linkExst) emit requestLinkExisting(serviceId);
    else if (chosen == simulate) emit requestSimulateFailure(serviceId);
}
