#include "EdgeLine.h"
#include "ServiceNode.h"
#include <QGraphicsPolygonItem>
#include <QGraphicsScene>
#include <QPolygonF>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QtMath>

EdgeLine::EdgeLine(ServiceNode* parentNode, ServiceNode* dependentNode,
                   int weight, bool absolute, QGraphicsItem* parent)
    : QGraphicsLineItem(parent),
    parentNode(parentNode), dependentNode(dependentNode),
    weight(weight), absolute(absolute)
{
    // color: red for absolute, black for weighted
    QPen pen(absolute ? QColor("#CC0000") : QColor("#222222"),
             absolute ? 2 : 1);
    setPen(pen);
    setZValue(0); // edges draw behind nodes

    // weight label — standalone scene item, positioned in reposition()
    weightLabel = new QGraphicsTextItem(QString::number(weight));
    QFont f = weightLabel->font();
    f.setPointSize(8);
    weightLabel->setFont(f);
    weightLabel->setDefaultTextColor(absolute ? QColor("#D0021B") : QColor("#555555"));
    weightLabel->setZValue(2); // above edges and nodes

    // arrowhead — standalone scene item, positioned in reposition()
    arrowHead = new QGraphicsPolygonItem();
    arrowHead->setBrush(QBrush(absolute ? QColor("#D0021B") : QColor("#555555")));
    arrowHead->setPen(Qt::NoPen);
    arrowHead->setZValue(0);

    // register with both nodes so they call reposition() when dragged
    parentNode->registerEdge(this);
    dependentNode->registerEdge(this);

    reposition();
}

EdgeLine::~EdgeLine() {
    // unregister from nodes so they don't call reposition() on a dead pointer
    parentNode->unregisterEdge(this);
    dependentNode->unregisterEdge(this);
    // weightLabel and arrowHead are scene items — MainWindow cleans them up
}

// -----------------------------------------------------------------------
// reposition — recompute line endpoints from current node centers,
//              update weight label and arrowhead
// -----------------------------------------------------------------------
void EdgeLine::reposition() {
    prepareGeometryChange(); // tell Qt the full dirty region is changing

    int r = ServiceNode::DIAMETER / 2;

    // center of each node in scene coordinates
    QPointF pCenter = parentNode->scenePos()    + QPointF(r, r);
    QPointF dCenter = dependentNode->scenePos() + QPointF(r, r);

    QPointF delta = dCenter - pCenter;
    double  len   = qSqrt(delta.x() * delta.x() + delta.y() * delta.y());
    if (len < 1.0) return;

    QPointF unit = delta / len;

    // line runs from circumference to circumference
    QPointF start = pCenter + unit * r;
    QPointF end   = dCenter - unit * r;
    setLine(QLineF(start, end));

    // weight label: midpoint with small perpendicular offset so it clears the line
    QPointF mid  = (start + end) / 2.0;
    QPointF perp = QPointF(-unit.y(), unit.x()) * 12.0;
    weightLabel->setPos(mid + perp - QPointF(8, 8));

    // arrowhead: small triangle pointing at dependent circumference
    const double arrowSize = 10.0;
    QPointF tip   = end;
    QPointF base  = tip - unit * arrowSize;
    QPointF left  = base + QPointF(-unit.y(),  unit.x()) * (arrowSize * 0.4);
    QPointF right = base + QPointF( unit.y(), -unit.x()) * (arrowSize * 0.4);

    QPolygonF poly;
    poly << tip << left << right;
    arrowHead->setPolygon(poly);
    if (scene()) scene()->update();
}
