#include "MainWindow.h"
#include "AddServiceDialog.h"
#include "AddEdgeDialog.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QWidget>
#include <QScrollBar>
#include <fstream>
#include <sstream>
#include <string>
#include <QtMath>

// =============================================================================
// Constructor / Destructor
// =============================================================================

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), manager(nullptr)
{
    setWindowTitle("Service Dependency Failure Simulator");
    resize(1100, 750);

    setupScene();
    setupMenuBar();
}

MainWindow::~MainWindow() {
    delete manager;
}

// =============================================================================
// setupScene — create the graphics scene and view
// =============================================================================
void MainWindow::setupScene() {
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 2000, 2000);
    scene->setBackgroundBrush(QColor("#B0B0B0")); // mid-grey, edges visible clearly

    view = new QGraphicsView(scene, this);
    view->setRenderHint(QPainter::Antialiasing);
    // NoDrag lets nodes handle their own movement without the view scrolling
    // underneath them and leaving shear trails
    view->setDragMode(QGraphicsView::NoDrag);
    view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    setCentralWidget(view);
}

// =============================================================================
// setupMenuBar
// =============================================================================
void MainWindow::setupMenuBar() {
    // --- File menu ---
    QMenu* fileMenu = menuBar()->addMenu("File");

    QAction* loadAct = fileMenu->addAction("Load from file...");
    connect(loadAct, &QAction::triggered, this, &MainWindow::onLoadFile);

    fileMenu->addSeparator();

    QAction* exitAct = fileMenu->addAction("Exit");
    connect(exitAct, &QAction::triggered, this, &QMainWindow::close);

    // --- Services menu ---
    QMenu* svcMenu = menuBar()->addMenu("Services");

    QAction* addAct  = svcMenu->addAction("Add new service");
    connect(addAct, &QAction::triggered, this, &MainWindow::onAddServiceFromMenu);

    QAction* listAct = svcMenu->addAction("List all services");
    connect(listAct, &QAction::triggered, this, &MainWindow::onListAllServices);

    // --- Simulate menu ---
    QMenu* simMenu = menuBar()->addMenu("Simulate");

    QAction* simAct   = simMenu->addAction("Simulate failure...");
    connect(simAct, &QAction::triggered, this, &MainWindow::onSimulateFailure);

    QAction* resetAct = simMenu->addAction("Reset simulation");
    connect(resetAct, &QAction::triggered, this, &MainWindow::onResetSimulation);
}

// =============================================================================
// loadFromFile — parse input file, build ServiceManager, draw canvas
// Reuses the same parsing logic as the original main.cpp but stores
// everything in our manager and draws nodes rather than printing to cout
// =============================================================================

// helper — trim whitespace from both ends
static std::string trimStr(const std::string& s) {
    int start = 0, end = (int)s.size() - 1;
    while (start <= end && (s[start] == ' ' || s[start] == '\t')) start++;
    while (end >= start && (s[end]   == ' ' || s[end]   == '\t')) end--;
    return s.substr(start, end - start + 1);
}

void MainWindow::loadFromFile(const QString& path) {
    std::ifstream file(path.toStdString());
    if (!file.is_open()) {
        QMessageBox::critical(this, "Error",
                              QString("Could not open file:\n%1").arg(path));
        return;
    }

    // clear existing state
    scene->clear();
    nodes.clear();
    delete manager;
    manager = nullptr;

    std::string line;
    bool headerRead = false;

    while (std::getline(file, line)) {
        line = trimStr(line);
        if (line.empty() || line[0] == '#') continue;

        if (!headerRead) {
            // first real line: MAX_SERVICES THRESHOLD MODE
            long long maxServices = 0;
            int threshold = 0, mode = 0, i = 0, n = (int)line.size();

            while (i < n && line[i] == ' ') i++;
            while (i < n && line[i] >= '0' && line[i] <= '9')
                maxServices = maxServices * 10 + (line[i++] - '0');
            while (i < n && line[i] == ' ') i++;
            while (i < n && line[i] >= '0' && line[i] <= '9')
                threshold = threshold * 10 + (line[i++] - '0');
            while (i < n && line[i] == ' ') i++;
            while (i < n && line[i] >= '0' && line[i] <= '9')
                mode = mode * 10 + (line[i++] - '0');

            manager = new ServiceManager(maxServices, threshold, mode);
            headerRead = true;
            continue;
        }

        // find command keyword
        int sp = 0;
        while (sp < (int)line.size() && line[sp] != ' ') sp++;
        std::string cmd  = line.substr(0, sp);
        std::string rest = trimStr(line.substr(sp));

        if (cmd == "SERVICE") {
            long long id = manager->addService(rest);
            if (id != -1)
                createNode(id, QString::fromStdString(rest));

        } else if (cmd == "EDGE") {
            long long parentId = 0, dependentId = 0;
            int weight = 0, absolute = 0, i = 0, n = (int)rest.size();

            while (i < n && rest[i] == ' ') i++;
            while (i < n && rest[i] >= '0' && rest[i] <= '9')
                parentId = parentId * 10 + (rest[i++] - '0');
            while (i < n && rest[i] == ' ') i++;
            while (i < n && rest[i] >= '0' && rest[i] <= '9')
                dependentId = dependentId * 10 + (rest[i++] - '0');
            while (i < n && rest[i] == ' ') i++;
            while (i < n && rest[i] >= '0' && rest[i] <= '9')
                weight = weight * 10 + (rest[i++] - '0');
            while (i < n && rest[i] == ' ') i++;
            while (i < n && rest[i] >= '0' && rest[i] <= '9')
                absolute = absolute * 10 + (rest[i++] - '0');

            manager->addEdge(parentId, dependentId, weight, absolute != 0);
            createEdge(parentId, dependentId, weight, absolute != 0);

        } else if (cmd == "SELF") {
            long long id = 0;
            int weight = 0, i = 0, n = (int)rest.size();

            while (i < n && rest[i] == ' ') i++;
            while (i < n && rest[i] >= '0' && rest[i] <= '9')
                id = id * 10 + (rest[i++] - '0');
            while (i < n && rest[i] == ' ') i++;
            while (i < n && rest[i] >= '0' && rest[i] <= '9')
                weight = weight * 10 + (rest[i++] - '0');

            manager->addEdge(id, id, weight, false);
            // self edges are not drawn on canvas — they're a logic concept
        }
    }

    file.close();
}

// =============================================================================
// createNode — add a ServiceNode to the scene, connect its signals
// =============================================================================
ServiceNode* MainWindow::createNode(long long id, const QString& name) {
    ServiceNode* node = new ServiceNode(id, name);
    node->setPos(autoPosition(id));
    scene->addItem(node);
    nodes[id] = node;

    // connect node right click signals to MainWindow slots
    connect(node, &ServiceNode::requestAddDependent,
            this, &MainWindow::onRequestAddDependent);
    connect(node, &ServiceNode::requestLinkExisting,
            this, &MainWindow::onRequestLinkExisting);
    connect(node, &ServiceNode::requestSimulateFailure,
            this, &MainWindow::onRequestSimulateFailure);

    return node;
}

// =============================================================================
// createEdge — draw an EdgeLine between two existing nodes
// =============================================================================
void MainWindow::createEdge(long long parentId, long long dependentId,
                            int weight, bool absolute) {
    if (!nodes.contains(parentId) || !nodes.contains(dependentId)) return;

    // self edges are skipped visually
    if (parentId == dependentId) return;

    ServiceNode* pNode = nodes[parentId];
    ServiceNode* dNode = nodes[dependentId];

    EdgeLine* edge = new EdgeLine(pNode, dNode, weight, absolute);
    scene->addItem(edge);
    scene->addItem(edge->getWeightLabel());
    scene->addItem(edge->getArrowHead());
}

// =============================================================================
// refreshNodeColors — recolor all nodes after a simulation run
// =============================================================================
void MainWindow::refreshNodeColors() {
    if (!manager) return;
    for (auto it = nodes.begin(); it != nodes.end(); ++it) {
        long long id = it.key();
        if (manager->isValidId(id)) {
            it.value()->setStatus(manager->getService(id).getStatus());
        }
    }
}

// =============================================================================
// autoPosition — spread nodes in a circle so they don't stack on load
// =============================================================================
QPointF MainWindow::autoPosition(long long id) const {
    // place nodes around a circle in the middle of the scene
    const double cx = 600, cy = 400, radius = 280;
    long long total = manager ? manager->getCapacity() : 10;
    double angle = (2.0 * M_PI * (id - 1)) / qMax(total, 1LL);
    return QPointF(cx + radius * qCos(angle), cy + radius * qSin(angle));
}

// =============================================================================
// buildServiceList — used by AddEdgeDialog dropdown
// =============================================================================
QList<QPair<long long, QString>> MainWindow::buildServiceList() const {
    QList<QPair<long long, QString>> list;
    if (!manager) return list;
    for (long long i = 1; i <= manager->getServiceCount(); i++) {
        list.append({i, QString::fromStdString(manager->getService(i).getName())});
    }
    return list;
}

// =============================================================================
// Menu bar slots
// =============================================================================

void MainWindow::onLoadFile() {
    QString path = QFileDialog::getOpenFileName(
        this, "Open services file", "", "Text files (*.txt);;All files (*)");
    if (!path.isEmpty())
        loadFromFile(path);
}

void MainWindow::onAddServiceFromMenu() {
    if (!manager) {
        QMessageBox::warning(this, "No project",
                             "Load a file first to set up the service graph.");
        return;
    }
    AddServiceDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;

    QString name = dlg.getServiceName();
    if (name.isEmpty()) return;

    long long id = manager->addService(name.toStdString());
    if (id != -1)
        createNode(id, name);
}

void MainWindow::onSimulateFailure() {
    if (!manager) return;

    // ask user for a service ID to fail
    bool ok;
    int id = QInputDialog::getInt(this, "Simulate Failure",
                                  "Enter service ID to fail:",
                                  1, 1, (int)manager->getServiceCount(), 1, &ok);
    if (!ok) return;

    manager->simulateFailure((long long)id);
    refreshNodeColors();
}

void MainWindow::onResetSimulation() {
    if (!manager) return;
    manager->resetSimulation();
    refreshNodeColors();
}

void MainWindow::onListAllServices() {
    if (!manager) {
        QMessageBox::information(this, "Services", "No project loaded.");
        return;
    }

    QString text;
    for (long long i = 1; i <= manager->getServiceCount(); i++) {
        const Service& svc = manager->getService(i);
        QString status;
        if      (svc.getStatus() == 0) status = "NORMAL";
        else if (svc.getStatus() == 1) status = "AFFECTED";
        else                           status = "FAILED";
        text += QString("[%1] %2 - %3\n")
                    .arg(i)
                    .arg(QString::fromStdString(svc.getName()))
                    .arg(status);
    }
    QMessageBox::information(this, "All Services", text);
}

// =============================================================================
// Node right click slots
// =============================================================================

void MainWindow::onRequestAddDependent(long long parentId) {
    if (!manager) return;

    AddServiceDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;

    QString name = dlg.getServiceName();
    if (name.isEmpty()) return;

    // get edge weight and absolute flag
    bool ok;
    int weight = QInputDialog::getInt(this, "Edge Weight",
                                      "Weight of dependency:", 1, 1, 100, 1, &ok);
    if (!ok) return;

    int absChoice = QMessageBox::question(this, "Absolute Edge",
                                          "Should this edge be absolute?\n"
                                          "(Dependent immediately fails when parent fails)",
                                          QMessageBox::Yes | QMessageBox::No);
    bool absolute = (absChoice == QMessageBox::Yes);

    // add service and edge to logic layer
    long long newId = manager->addService(name.toStdString());
    if (newId == -1) return;

    manager->addEdge(parentId, newId, weight, absolute);

    // draw on canvas
    createNode(newId, name);
    createEdge(parentId, newId, weight, absolute);
}

void MainWindow::onRequestLinkExisting(long long parentId) {
    if (!manager) return;

    AddEdgeDialog dlg(buildServiceList(), parentId, this);
    if (dlg.exec() != QDialog::Accepted) return;

    long long targetId = dlg.getTargetId();
    if (targetId == -1) return;

    int  weight   = dlg.getWeight();
    bool absolute = dlg.getAbsolute();

    manager->addEdge(parentId, targetId, weight, absolute);
    createEdge(parentId, targetId, weight, absolute);
}

void MainWindow::onRequestSimulateFailure(long long serviceId) {
    if (!manager) return;
    manager->simulateFailure(serviceId);
    refreshNodeColors();
}
