#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMap>
#include "ServiceManager.h"
#include "ServiceNode.h"
#include "EdgeLine.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onNewProject();
    void onLoadFile();
    void onSaveFile();
    void onAddServiceFromMenu();
    void onSimulateFailure();
    void onResetSimulation();
    void onListByStatus(int status);

    // node right-click signals
    void onRequestAddDependent(long long parentId);
    void onRequestLinkExisting(long long parentId);
    void onRequestSimulateFailure(long long serviceId);

private:
    QGraphicsScene*               scene;
    QGraphicsView*                view;
    ServiceManager*               manager;
    QMap<long long, ServiceNode*> nodes;

    void        setupMenuBar();
    void        setupScene();
    void        loadFromFile(const QString& path);
    ServiceNode* createNode(long long id, const QString& name);
    void        createEdge(long long parentId, long long dependentId,
                           int weight, bool absolute);
    void        refreshNodeColors();
    void        refreshAllWeightDisplays();
    QPointF     autoPosition(long long id) const;
};

#endif
