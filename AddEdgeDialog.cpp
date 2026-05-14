#include "AddEdgeDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QButtonGroup>

AddEdgeDialog::AddEdgeDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Link to Existing Service");
    setMinimumWidth(300);

    QVBoxLayout* layout = new QVBoxLayout(this);

    // --- selection mode ---
    QHBoxLayout* modeRow = new QHBoxLayout();
    byIdRadio   = new QRadioButton("By ID",   this);
    byNameRadio = new QRadioButton("By Name", this);
    byIdRadio->setChecked(true);
    QButtonGroup* group = new QButtonGroup(this);
    group->addButton(byIdRadio);
    group->addButton(byNameRadio);
    modeRow->addWidget(new QLabel("Select by:"));
    modeRow->addWidget(byIdRadio);
    modeRow->addWidget(byNameRadio);
    modeRow->addStretch();
    layout->addLayout(modeRow);

    // --- target input ---
    QFormLayout* form = new QFormLayout();
    targetEdit = new QLineEdit(this);
    targetEdit->setPlaceholderText("Enter ID or name...");
    form->addRow("Target service:", targetEdit);

    weightSpin = new QSpinBox(this);
    weightSpin->setRange(1, 100);
    weightSpin->setValue(1);
    form->addRow("Edge weight:", weightSpin);

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
    connect(targetEdit, &QLineEdit::returnPressed, this, &QDialog::accept);
}

bool AddEdgeDialog::isByName() const {
    return byNameRadio->isChecked();
}

long long AddEdgeDialog::getTargetId() const {
    bool ok;
    long long id = (long long)targetEdit->text().trimmed().toLongLong(&ok);
    return ok ? id : -1;
}

QString AddEdgeDialog::getTargetName() const {
    return targetEdit->text().trimmed();
}

int AddEdgeDialog::getWeight() const {
    return weightSpin->value();
}

bool AddEdgeDialog::getAbsolute() const {
    return absoluteCheck->isChecked();
}
