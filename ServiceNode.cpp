#include "ServiceNode.h"
#include "EdgeLine.h"
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QAction>
#include <QBrush>
#include <QPen>
#include <QFont>
#include <QMessageBox>

ServiceNode::ServiceNode(long long serviceId, const QString& name,
                         QGraphicsItem* parent)
    : QObject(), QGraphicsEllipseItem(0, 0, DIAMETER, DIAMETER, parent),
      serviceId(serviceId), failedWeight(0.0f), totalWeight(0)
{
    updateColor(0);
    setPen(QPen(Qt::black, 2));

    // service name centered in the circle
    nameLabel = new QGraphicsTextItem(name, this);
    QFont f = nameLabel->font();
    f.setPointSize(8);
    f.setBold(true);
    nameLabel->setFont(f);
    QRectF b = nameLabel->boundingRect();
    nameLabel->setPos((DIAMETER - b.width()) / 2.0,
                      (DIAMETER - b.height()) / 2.0);

    // self-weight label - bottom-right inside the circle, hidden until set
    selfWeightLabel = new QGraphicsTextItem("", this);
    QFont sf = selfWeightLabel->font();
    sf.setPointSize(7);
    selfWeightLabel->setFont(sf);
    selfWeightLabel->setDefaultTextColor(QColor("#003366"));
    selfWeightLabel->setPos(DIAMETER * 0.55, DIAMETER * 0.65);
    selfWeightLabel->hide();

    setFlag(QGraphicsItem::ItemIsMovable,           true);
    setFlag(QGraphicsItem::ItemIsSelectable,        true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges,true);
    setZValue(1);
}

void ServiceNode::setStatus(int status) {
    updateColor(status);
}

void ServiceNode::updateColor(int status) {
    if      (status == 0) setBrush(QBrush(QColor("#4A90D9"))); // normal  - blue
    else if (status == 1) setBrush(QBrush(QColor("#F5A623"))); // affected - yellow
    else                  setBrush(QBrush(QColor("#D0021B"))); // failed  - red
}

// setSelfWeight - display self-resilience weight inside the node
// "o" prefix distinguishes it from normal edge weights visually
void ServiceNode::setSelfWeight(int weight) {
    selfWeightLabel->setPlainText(QString("o%1").arg(weight));
    selfWeightLabel->show();
}

// updateWeightDisplay - cache latest weight data for the click summary
void ServiceNode::updateWeightDisplay(float fw, int tw) {
    failedWeight = fw;
    totalWeight  = tw;
}

void ServiceNode::registerEdge(EdgeLine* edge) {
    if (!edges.contains(edge)) edges.append(edge);
}

void ServiceNode::unregisterEdge(EdgeLine* edge) {
    edges.removeAll(edge);
}

QVariant ServiceNode::itemChange(GraphicsItemChange change,
                                  const QVariant& value) {
    if (change == ItemPositionHasChanged) {
        for (EdgeLine* edge : edges) edge->reposition();
    }
    return QGraphicsEllipseItem::itemChange(change, value);
}

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
