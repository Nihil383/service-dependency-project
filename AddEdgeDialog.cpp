#include "AddEdgeDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>

AddEdgeDialog::AddEdgeDialog(const QList<QPair<long long, QString>>& services,
                             long long excludeId,
                             QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Link to Existing Service");
    setMinimumWidth(300);

    QVBoxLayout* layout = new QVBoxLayout(this);
    QFormLayout* form   = new QFormLayout();

    // --- service dropdown ---
    serviceCombo = new QComboBox(this);
    for (const auto& pair : services) {
        if (pair.first == excludeId) continue; // don't show the source node
        serviceCombo->addItem(
            QString("[%1] %2").arg(pair.first).arg(pair.second)
        );
        idList.append(pair.first);
    }
    form->addRow("Dependent service:", serviceCombo);

    // --- weight spinbox ---
    weightSpin = new QSpinBox(this);
    weightSpin->setRange(1, 100);
    weightSpin->setValue(1);
    form->addRow("Edge weight:", weightSpin);

    // --- absolute checkbox ---
    absoluteCheck = new QCheckBox("Absolute (dependent immediately fails)", this);
    form->addRow("", absoluteCheck);

    layout->addLayout(form);

    // --- buttons ---
    QHBoxLayout* btnRow = new QHBoxLayout();
    QPushButton* ok     = new QPushButton("Link",   this);
    QPushButton* cancel = new QPushButton("Cancel", this);
    btnRow->addStretch();
    btnRow->addWidget(ok);
    btnRow->addWidget(cancel);
    layout->addLayout(btnRow);

    connect(ok,     &QPushButton::clicked, this, &QDialog::accept);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
}

long long AddEdgeDialog::getTargetId() const {
    int idx = serviceCombo->currentIndex();
    if (idx < 0 || idx >= idList.size()) return -1;
    return idList[idx];
}

int AddEdgeDialog::getWeight() const {
    return weightSpin->value();
}

bool AddEdgeDialog::getAbsolute() const {
    return absoluteCheck->isChecked();
}
