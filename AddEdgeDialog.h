#ifndef ADDEDGEDIALOG_H
#define ADDEDGEDIALOG_H

#include <QDialog>
#include <QRadioButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>

// =============================================================================
// AddEdgeDialog - collects info to link two existing services
//
// Changed from original:
//   Replaced the QComboBox dropdown with a radio button pair (By ID / By Name)
//   and a single QLineEdit. The user types the target service's ID or name
//   directly, which is more usable when the graph gets large.
// =============================================================================
class AddEdgeDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddEdgeDialog(QWidget* parent = nullptr);

    // returns typed ID (if By ID selected) or -1 if By Name
    // call getTargetName() in that case and resolve via ServiceManager
    bool      isByName()      const;
    long long getTargetId()   const;
    QString   getTargetName() const;
    int       getWeight()     const;
    bool      getAbsolute()   const;

private:
    QRadioButton* byIdRadio;
    QRadioButton* byNameRadio;
    QLineEdit*    targetEdit;
    QSpinBox*     weightSpin;
    QCheckBox*    absoluteCheck;
};

#endif
