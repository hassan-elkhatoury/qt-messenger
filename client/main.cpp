#include "loginwindow.h"

#include <QApplication>
#include <QFile>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application information
    QApplication::setApplicationName("QtMessenger");
    QApplication::setApplicationDisplayName("Qt Messenger");
    QApplication::setOrganizationName("QtMessenger");
    
    // Load default style (light theme)
    QFile styleFile(":/resources/light.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = styleFile.readAll();
        app.setStyleSheet(style);
        styleFile.close();
    }
    
    // Create and show login window
    LoginWindow loginWindow;
    loginWindow.show();
    
    return app.exec();
}