#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>

class NetworkClient : public QObject
{
    Q_OBJECT

public:
    explicit NetworkClient(QObject *parent = nullptr);
    ~NetworkClient();
    
    void sendRequest(const QJsonObject &request);
    void disconnect();
    bool isConnected() const;

signals:
    void responseReceived(const QJsonObject &response);
    void connectionError(const QString &errorMessage);
    void messageReceived(const QJsonObject &message);
    void connected();
    void disconnected();

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);
    void attemptReconnect();

private:
    QTcpSocket *socket;
    QTimer *reconnectTimer;
    bool reconnecting;
    
    void connectToServer();
    QJsonObject createErrorResponse(const QString &message);
    
    // Default server settings (should be configurable)
    QString serverHost = "localhost";
    quint16 serverPort = 8080;
};

#endif // NETWORKCLIENT_H