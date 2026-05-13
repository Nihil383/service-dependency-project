#ifndef ADDSERVICEDIALOG_H
#define ADDSERVICEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QString>

// =============================================================================
// AddServiceDialog — tiny dialog that asks for a service name
// Used when: adding a brand new service from the canvas or menu
// =============================================================================
class AddServiceDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddServiceDialog(QWidget* parent = nullptr);

    // returns the name the user typed, or empty string if cancelled
    QString getServiceName() const;

private:
    QLineEdit* nameEdit;
};

#endif
