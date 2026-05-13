#include "AddServiceDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

AddServiceDialog::AddServiceDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Add Service");
    setMinimumWidth(280);

    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel("Service name:"));
    nameEdit = new QLineEdit(this);
    nameEdit->setPlaceholderText("e.g. Database");
    layout->addWidget(nameEdit);

    // OK / Cancel buttons
    QHBoxLayout* btnRow = new QHBoxLayout();
    QPushButton* ok     = new QPushButton("Add",    this);
    QPushButton* cancel = new QPushButton("Cancel", this);
    btnRow->addStretch();
    btnRow->addWidget(ok);
    btnRow->addWidget(cancel);
    layout->addLayout(btnRow);

    connect(ok,     &QPushButton::clicked, this, &QDialog::accept);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);

    // pressing Enter in the name field also accepts
    connect(nameEdit, &QLineEdit::returnPressed, this, &QDialog::accept);
}

QString AddServiceDialog::getServiceName() const {
    return nameEdit->text().trimmed();
}
