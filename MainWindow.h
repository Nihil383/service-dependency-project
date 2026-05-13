#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMap>
#include "ServiceManager.h"
#include "ServiceNode.h"
#include "EdgeLine.h"

// =============================================================================
// MainWindow — top level window
//
// Owns:
//   scene         — QGraphicsScene where nodes and edges live
//   view          — QGraphicsView that renders the scene
//   manager       — ServiceManager (the logic layer, untouched)
//   nodes         — map of serviceId → ServiceNode* for quick canvas lookup
//
// Layout:
//   Menu bar      — File menu, Simulate menu
//   Central widget— QGraphicsView (the canvas)
// =============================================================================
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // -----------------------------------------------------------------------
    // Menu bar actions
    // -----------------------------------------------------------------------
    void onLoadFile();
    void onAddServiceFromMenu();
    void onSimulateFailure();
    void onResetSimulation();
    void onListAllServices();

    // -----------------------------------------------------------------------
    // Node right click signals — connected when a node is created
    // -----------------------------------------------------------------------
    void onRequestAddDependent(long long parentId);
    void onRequestLinkExisting(long long parentId);
    void onRequestSimulateFailure(long long serviceId);

private:
    QGraphicsScene*  scene;
    QGraphicsView*   view;
    ServiceManager*  manager;

    // serviceId → node pointer so we can find nodes by ID quickly
    QMap<long long, ServiceNode*> nodes;

    // -----------------------------------------------------------------------
    // helpers
    // -----------------------------------------------------------------------

    // create a ServiceManager from a file, rebuild the canvas from it
    void loadFromFile(const QString& path);

    // add one node to the scene at a sensible auto position
    // called both on file load and on runtime add
    ServiceNode* createNode(long long id, const QString& name);

    // draw an edge line between two existing nodes
    void createEdge(long long parentId, long long dependentId,
                    int weight, bool absolute);

    // after a simulateFailure call, walk all nodes and recolor them
    void refreshNodeColors();

    // build the list of (id, name) pairs the AddEdgeDialog needs
    QList<QPair<long long, QString>> buildServiceList() const;

    // auto-layout: place node i in a rough circle so they don't stack
    QPointF autoPosition(long long id) const;

    void setupMenuBar();
    void setupScene();
};

#endif
