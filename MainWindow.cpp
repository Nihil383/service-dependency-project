#include "MainWindow.h"
#include "AddServiceDialog.h"
#include "AddEdgeDialog.h"
#include "Service.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <fstream>
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
// setupScene
// =============================================================================
void MainWindow::setupScene() {
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 2000, 2000);
    scene->setBackgroundBrush(QColor("#B0B0B0"));

    view = new QGraphicsView(scene, this);
    view->setRenderHint(QPainter::Antialiasing);
    view->setDragMode(QGraphicsView::NoDrag);
    view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setCentralWidget(view);
}

// =============================================================================
// setupMenuBar
// =============================================================================
void MainWindow::setupMenuBar() {
    // --- File ---
    QMenu* fileMenu = menuBar()->addMenu("File");

    QAction* newAct  = fileMenu->addAction("New project...");
    connect(newAct, &QAction::triggered, this, &MainWindow::onNewProject);

    QAction* loadAct = fileMenu->addAction("Load from file...");
    connect(loadAct, &QAction::triggered, this, &MainWindow::onLoadFile);

    QAction* saveAct = fileMenu->addAction("Save...");
    connect(saveAct, &QAction::triggered, this, &MainWindow::onSaveFile);

    fileMenu->addSeparator();
    QAction* exitAct = fileMenu->addAction("Exit");
    connect(exitAct, &QAction::triggered, this, &QMainWindow::close);

    // --- Services ---
    QMenu* svcMenu = menuBar()->addMenu("Services");

    QAction* addAct = svcMenu->addAction("Add new service");
    connect(addAct, &QAction::triggered, this, &MainWindow::onAddServiceFromMenu);

    // filtered listing submenu
    QMenu* listMenu      = svcMenu->addMenu("List services by status");
    QAction* listNormal   = listMenu->addAction("Normal");
    QAction* listAffected = listMenu->addAction("Affected");
    QAction* listFailed   = listMenu->addAction("Failed");
    connect(listNormal,   &QAction::triggered, this, [this]{ onListByStatus(0); });
    connect(listAffected, &QAction::triggered, this, [this]{ onListByStatus(1); });
    connect(listFailed,   &QAction::triggered, this, [this]{ onListByStatus(2); });

    // --- Simulate ---
    QMenu* simMenu = menuBar()->addMenu("Simulate");

    QAction* simAct   = simMenu->addAction("Simulate failure...");
    connect(simAct, &QAction::triggered, this, &MainWindow::onSimulateFailure);

    QAction* resetAct = simMenu->addAction("Reset simulation");
    connect(resetAct, &QAction::triggered, this, &MainWindow::onResetSimulation);
}

// =============================================================================
// File parsing helper
// =============================================================================
static std::string trimStr(const std::string& s) {
    int start = 0, end = (int)s.size() - 1;
    while (start <= end && (s[start]==' '||s[start]=='\t')) start++;
    while (end >= start && (s[end]  ==' '||s[end]  =='\t')) end--;
    return s.substr(start, end - start + 1);
}

// =============================================================================
// loadFromFile
// =============================================================================
void MainWindow::loadFromFile(const QString& path) {
    std::ifstream file(path.toStdString());
    if (!file.is_open()) {
        QMessageBox::critical(this, "Error",
            QString("Could not open file:\n%1").arg(path));
        return;
    }

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
            // first real line: THRESHOLD MODE (no max services anymore)
            // for backwards compatibility still accept the old 3-number format
            // and just ignore the first number if present
            int threshold = 50, mode = 1;
            int i = 0, n = (int)line.size();
            long long first = 0, second = 0, third = 0;
            int tokenCount = 0;

            while (i < n) {
                while (i < n && line[i] == ' ') i++;
                if (i >= n || line[i] < '0' || line[i] > '9') break;
                long long val = 0;
                while (i < n && line[i] >= '0' && line[i] <= '9')
                    val = val * 10 + (line[i++] - '0');
                tokenCount++;
                if      (tokenCount == 1) first  = val;
                else if (tokenCount == 2) second = val;
                else if (tokenCount == 3) third  = val;
            }

            if (tokenCount == 3) {
                // old format: MAX THRESHOLD MODE - ignore MAX
                threshold = (int)second;
                mode      = (int)third;
            } else if (tokenCount == 2) {
                // new format: THRESHOLD MODE
                threshold = (int)first;
                mode      = (int)second;
            }

            manager = new ServiceManager(threshold, mode);
            headerRead = true;
            continue;
        }

        int sp = 0;
        while (sp < (int)line.size() && line[sp] != ' ') sp++;
        std::string cmd  = line.substr(0, sp);
        std::string rest = trimStr(line.substr(sp));

        if (cmd == "SERVICE") {
            long long id = manager->addService(rest);
            if (id != -1) createNode(id, QString::fromStdString(rest));

        } else if (cmd == "EDGE") {
            long long parentId = 0, dependentId = 0;
            int weight = 0, absolute = 0, i = 0, n = (int)rest.size();
            while (i<n&&rest[i]==' ') i++;
            while (i<n&&rest[i]>='0'&&rest[i]<='9') parentId=parentId*10+(rest[i++]-'0');
            while (i<n&&rest[i]==' ') i++;
            while (i<n&&rest[i]>='0'&&rest[i]<='9') dependentId=dependentId*10+(rest[i++]-'0');
            while (i<n&&rest[i]==' ') i++;
            while (i<n&&rest[i]>='0'&&rest[i]<='9') weight=weight*10+(rest[i++]-'0');
            while (i<n&&rest[i]==' ') i++;
            while (i<n&&rest[i]>='0'&&rest[i]<='9') absolute=absolute*10+(rest[i++]-'0');

            if (manager->addEdge(parentId, dependentId, weight, absolute != 0))
                createEdge(parentId, dependentId, weight, absolute != 0);

        } else if (cmd == "SELF") {
            long long id = 0;
            int weight = 0, i = 0, n = (int)rest.size();
            while (i<n&&rest[i]==' ') i++;
            while (i<n&&rest[i]>='0'&&rest[i]<='9') id=id*10+(rest[i++]-'0');
            while (i<n&&rest[i]==' ') i++;
            while (i<n&&rest[i]>='0'&&rest[i]<='9') weight=weight*10+(rest[i++]-'0');

            if (manager->addEdge(id, id, weight, false)) {
                // self-edge: update the node's self-weight display
                if (nodes.contains(id)) nodes[id]->setSelfWeight(weight);
            }
        }
    }

    file.close();
    refreshAllWeightDisplays();
}

// =============================================================================
// createNode
// =============================================================================
ServiceNode* MainWindow::createNode(long long id, const QString& name) {
    ServiceNode* node = new ServiceNode(id, name);
    node->setPos(autoPosition(id));
    scene->addItem(node);
    nodes[id] = node;

    connect(node, &ServiceNode::requestAddDependent,
            this, &MainWindow::onRequestAddDependent);
    connect(node, &ServiceNode::requestLinkExisting,
            this, &MainWindow::onRequestLinkExisting);
    connect(node, &ServiceNode::requestSimulateFailure,
            this, &MainWindow::onRequestSimulateFailure);

    return node;
}

// =============================================================================
// createEdge
// =============================================================================
void MainWindow::createEdge(long long parentId, long long dependentId,
                             int weight, bool absolute) {
    if (!nodes.contains(parentId) || !nodes.contains(dependentId)) return;
    if (parentId == dependentId) return; // self-edges shown in node, not as lines

    ServiceNode* pNode = nodes[parentId];
    ServiceNode* dNode = nodes[dependentId];

    EdgeLine* edge = new EdgeLine(pNode, dNode, weight, absolute);
    scene->addItem(edge);
    scene->addItem(edge->getWeightLabel());
    scene->addItem(edge->getArrowHead());
}

// =============================================================================
// refreshNodeColors - called after simulation to recolor all nodes
// =============================================================================
void MainWindow::refreshNodeColors() {
    if (!manager) return;
    for (auto it = nodes.begin(); it != nodes.end(); ++it) {
        long long id = it.key();
        if (manager->isValidId(id))
            it.value()->setStatus(manager->getService(id).getStatus());
    }
}

// =============================================================================
// refreshAllWeightDisplays - sync cached weight data into every node
// called after simulation and after adding edges
// =============================================================================
void MainWindow::refreshAllWeightDisplays() {
    if (!manager) return;
    for (auto it = nodes.begin(); it != nodes.end(); ++it) {
        long long id = it.key();
        if (manager->isValidId(id)) {
            const Service& svc = manager->getService(id);
            it.value()->updateWeightDisplay(
                svc.getTotalFailedWeight(),
                svc.getTotalWeight()
            );
        }
    }
}

// =============================================================================
// autoPosition - circular layout for initial node placement
// =============================================================================
QPointF MainWindow::autoPosition(long long id) const {
    const double cx = 600, cy = 400, radius = 280;
    long long total = manager ? qMax(manager->getServiceCount(), 1LL) : 10;
    double angle = (2.0 * M_PI * (id - 1)) / total;
    return QPointF(cx + radius * qCos(angle), cy + radius * qSin(angle));
}

// =============================================================================
// Menu slots
// =============================================================================

void MainWindow::onNewProject() {
    bool ok;
    int threshold = QInputDialog::getInt(this, "New Project",
        "Failure threshold (0-100%):", 50, 0, 100, 1, &ok);
    if (!ok) return;

    int modeIdx = QInputDialog::getItem(this, "New Project",
        "Propagation mode:",
        QStringList() << "0 - Naive" << "1 - Weighted",
        1, false, &ok).left(1).toInt();
    if (!ok) return;

    scene->clear();
    nodes.clear();
    delete manager;
    manager = new ServiceManager(threshold, modeIdx);
}

void MainWindow::onLoadFile() {
    QString path = QFileDialog::getOpenFileName(
        this, "Open services file", "", "Text files (*.txt);;All files (*)");
    if (!path.isEmpty()) loadFromFile(path);
}

void MainWindow::onSaveFile() {
    if (!manager) {
        QMessageBox::warning(this, "Nothing to save", "No project loaded.");
        return;
    }

    QString path = QFileDialog::getSaveFileName(
        this, "Save project", "", "Text files (*.txt);;All files (*)");
    if (path.isEmpty()) return;

    std::ofstream file(path.toStdString());
    if (!file.is_open()) {
        QMessageBox::critical(this, "Error",
            QString("Could not write to:\n%1").arg(path));
        return;
    }

    // new two-number header format: THRESHOLD MODE
    file << manager->getFailureThreshold() << " "
         << manager->getAffectMode() << "\n\n";

    for (long long i = 1; i <= manager->getServiceCount(); i++)
        file << "SERVICE " << manager->getService(i).getName() << "\n";
    file << "\n";

    for (long long i = 1; i <= manager->getServiceCount(); i++) {
        const EdgeList& edges = manager->getEdgesFrom(i);
        EdgeNode* e = edges.getHead();
        while (e) {
            if (e->dependentId == i)
                file << "SELF " << i << " " << e->weight << "\n";
            else
                file << "EDGE " << i << " " << e->dependentId
                     << " " << e->weight << " " << (e->absolute ? 1 : 0) << "\n";
            e = e->next;
        }
    }

    file.close();
    QMessageBox::information(this, "Saved",
        QString("Saved to:\n%1").arg(path));
}

void MainWindow::onAddServiceFromMenu() {
    if (!manager) {
        QMessageBox::warning(this, "No project",
            "Create or load a project first.");
        return;
    }
    AddServiceDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    QString name = dlg.getServiceName();
    if (name.isEmpty()) return;
    long long id = manager->addService(name.toStdString());
    if (id != -1) createNode(id, name);
}

void MainWindow::onSimulateFailure() {
    if (!manager) return;
    bool ok;
    int id = QInputDialog::getInt(this, "Simulate Failure",
        "Enter service ID to fail:",
        1, 1, (int)manager->getServiceCount(), 1, &ok);
    if (!ok) return;
    manager->simulateFailure((long long)id);
    refreshNodeColors();
    refreshAllWeightDisplays();
}

void MainWindow::onResetSimulation() {
    if (!manager) return;
    manager->resetSimulation();
    refreshNodeColors();
    refreshAllWeightDisplays();
}

void MainWindow::onListByStatus(int status) {
    if (!manager) {
        QMessageBox::information(this, "Services", "No project loaded.");
        return;
    }

    QString title;
    const IDList* list = nullptr;
    if      (status == STATUS_NORMAL)   { title = "Normal Services";   list = &manager->getNormalList();   }
    else if (status == STATUS_AFFECTED) { title = "Affected Services";  list = &manager->getAffectedList(); }
    else                                { title = "Failed Services";    list = &manager->getFailedList();   }

    QString text;
    IDListNode* curr = list->getHead();
    while (curr) {
        const Service& svc = manager->getService(curr->id);
        text += QString("[%1] %2").arg(curr->id)
                                  .arg(QString::fromStdString(svc.getName()));
        if (svc.getTotalWeight() > 0) {
            int pct = (int)(svc.failRatio() * 100.0f + 0.5f);
            text += QString(" (%1% load)").arg(pct);
        }
        text += "\n";
        curr = curr->next;
    }
    if (text.isEmpty()) text = "No services in this state.";
    QMessageBox::information(this, title, text);
}

// =============================================================================
// Node right-click slots
// =============================================================================

void MainWindow::onRequestAddDependent(long long parentId) {
    if (!manager) return;

    AddServiceDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    QString name = dlg.getServiceName();
    if (name.isEmpty()) return;

    bool ok;
    int weight = QInputDialog::getInt(this, "Edge Weight",
        "Weight of dependency:", 1, 1, 100, 1, &ok);
    if (!ok) return;

    int absChoice = QMessageBox::question(this, "Absolute Edge",
        "Should this edge be absolute?\n"
        "(Dependent immediately fails when parent fully fails)",
        QMessageBox::Yes | QMessageBox::No);
    bool absolute = (absChoice == QMessageBox::Yes);

    long long newId = manager->addService(name.toStdString());
    if (newId == -1) return;

    if (manager->addEdge(parentId, newId, weight, absolute)) {
        createNode(newId, name);
        createEdge(parentId, newId, weight, absolute);
        refreshAllWeightDisplays();
    }
}

void MainWindow::onRequestLinkExisting(long long parentId) {
    if (!manager) return;

    AddEdgeDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;

    long long targetId = -1;
    if (dlg.isByName()) {
        targetId = manager->searchByName(dlg.getTargetName().toStdString());
        if (targetId == -1) {
            QMessageBox::warning(this, "Not Found",
                QString("No service named '%1'.").arg(dlg.getTargetName()));
            return;
        }
    } else {
        targetId = dlg.getTargetId();
        if (!manager->isValidId(targetId)) {
            QMessageBox::warning(this, "Invalid ID",
                QString("No service with ID %1.").arg(targetId));
            return;
        }
    }

    int  weight   = dlg.getWeight();
    bool absolute = dlg.getAbsolute();

    // self-edge path
    if (targetId == parentId) {
        if (manager->addEdge(parentId, targetId, weight, false)) {
            if (nodes.contains(parentId))
                nodes[parentId]->setSelfWeight(weight);
            refreshAllWeightDisplays();
        } else {
            QMessageBox::information(this, "Duplicate",
                "A self-edge already exists for this service.");
        }
        return;
    }

    if (manager->addEdge(parentId, targetId, weight, absolute)) {
        createEdge(parentId, targetId, weight, absolute);
        refreshAllWeightDisplays();
    } else {
        QMessageBox::information(this, "Duplicate",
            "That edge already exists.");
    }
}

void MainWindow::onRequestSimulateFailure(long long serviceId) {
    if (!manager) return;
    manager->simulateFailure(serviceId);
    refreshNodeColors();
    refreshAllWeightDisplays();
}
