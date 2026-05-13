#include <QApplication>
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName("Service Dependency Failure Simulator");
    app.setOrganizationName("Student Project");

    MainWindow window;
    window.show();

    return app.exec();
}
