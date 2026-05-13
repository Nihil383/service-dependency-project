#ifndef ADDEDGEDIALOG_H
#define ADDEDGEDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QStringList>

// =============================================================================
// AddEdgeDialog — dialog for adding an edge between two existing services
//
// Used when: "Link to existing service" is chosen from a node's right click menu
//
// Collects:
//   targetId  — the ID of the service to link to (chosen from a dropdown)
//   weight    — edge weight (spinbox, 1–100)
//   absolute  — whether the edge is absolute (checkbox)
// =============================================================================
class AddEdgeDialog : public QDialog {
    Q_OBJECT

public:
    // serviceNames is a list of "ID: Name" strings for the dropdown
    // excludeId is the node that was right-clicked — excluded from the list
    explicit AddEdgeDialog(const QList<QPair<long long, QString>>& services,
                           long long excludeId,
                           QWidget* parent = nullptr);

    long long getTargetId() const;
    int       getWeight()   const;
    bool      getAbsolute() const;

private:
    QComboBox* serviceCombo;
    QSpinBox*  weightSpin;
    QCheckBox* absoluteCheck;

    // parallel list to combo so we can map selection index → service ID
    QList<long long> idList;
};

#endif
