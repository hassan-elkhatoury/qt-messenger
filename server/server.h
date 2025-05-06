#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMap>

#include "database.h"

class Server : public QObject
{
    Q_OBJECT

public:
    explicit Server(quint16 port, Database *database, QObject *parent = nullptr);
    ~Server();

    bool start();
    void stop();

private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onReadyRead();

private:
    void handleRequest(QTcpSocket *client, const QJsonObject &request);
    void handleLogin(QTcpSocket *client, const QJsonObject &request);
    void handleRegister(QTcpSocket *client, const QJsonObject &request);
    void handleGetContacts(QTcpSocket *client, const QJsonObject &request);
    void handleGetChatHistory(QTcpSocket *client, const QJsonObject &request);
    void handleSendMessage(QTcpSocket *client, const QJsonObject &request);
    void handleAddContact(QTcpSocket *client, const QJsonObject &request);

    void sendResponse(QTcpSocket *client, const QJsonObject &response);
    void broadcastToUser(int userId, const QJsonObject &message);

    QTcpServer *server;
    Database *database;

    QMap<int, QTcpSocket*> userConnections;      // userId -> socket
    QMap<QTcpSocket*, int> socketUsers;          // socket -> userId
};

#endif // SERVER_H
