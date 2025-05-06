#include "server.h"
#include "database.h"

#include <QCoreApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    app.setApplicationName("QtMessengerServer");
    app.setApplicationVersion("1.0.0");
    
    QCommandLineParser parser;
    parser.setApplicationDescription("Qt Messenger Server");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption portOption(QStringList() << "p" << "port",
                                 "Specify server port (default: 8080).",
                                 "port", "8080");
    parser.addOption(portOption);
    
    parser.process(app);
    
    quint16 port = parser.value(portOption).toUShort();
    
    // Initialize database
    Database db;
    if (!db.initialize()) {
        qCritical() << "Failed to initialize database!";
        return 1;
    }
    
    // Create and start server
    Server server(port, &db);
    if (!server.start()) {
        qCritical() << "Failed to start server!";
        return 1;
    }
    
    qInfo() << "Server is running on port" << port;
    
    return app.exec();
}