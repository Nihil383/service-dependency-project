#ifndef EDGELINE_H
#define EDGELINE_H

#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGraphicsPolygonItem>

class ServiceNode;  // forward declare

// =============================================================================
// EdgeLine — one directed line on the canvas representing a dependency edge
//
// Holds pointers to its parent and dependent ServiceNode so it can
// recompute its endpoints whenever either node is dragged.
//
// Visual rules:
//   absolute == true  → red line,   pen width 2
//   absolute == false → black line, pen width 1
//
// Weight is shown as a small label at the midpoint of the line.
// An arrowhead is drawn at the dependent end to show direction.
// =============================================================================
class EdgeLine : public QGraphicsLineItem {
public:
    EdgeLine(ServiceNode* parentNode, ServiceNode* dependentNode,
             int weight, bool absolute, QGraphicsItem* parent = nullptr);

    ~EdgeLine();

    // -----------------------------------------------------------------------
    // reposition — recomputes line from current center of both nodes
    // Called by ServiceNode::itemChange whenever either endpoint moves
    // -----------------------------------------------------------------------
    void reposition();

    ServiceNode*          getParentNode()    const { return parentNode; }
    ServiceNode*          getDependentNode() const { return dependentNode; }
    QGraphicsTextItem*    getWeightLabel()   const { return weightLabel; }
    QGraphicsPolygonItem* getArrowHead()     const { return arrowHead; }

private:
    ServiceNode*       parentNode;
    ServiceNode*       dependentNode;
    int                weight;
    bool               absolute;
    QGraphicsTextItem* weightLabel;

    // draws a small triangle arrowhead near the dependent node
    void updateArrow();
    QGraphicsPolygonItem* arrowHead;
};

#endif
