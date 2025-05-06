#include "networkclient.h"
#include <QHostAddress>
#include <QSettings>

NetworkClient::NetworkClient(QObject *parent)
    : QObject(parent)
    , socket(new QTcpSocket(this))
    , reconnectTimer(new QTimer(this))
    , reconnecting(false)
    , serverHost("127.0.0.1")  // Changed from "localhost" to explicit IP
    , serverPort(8080)         // Make sure this matches your server's port
{
    // Configure reconnect timer
    reconnectTimer->setInterval(5000); // 5 seconds between reconnect attempts

    // Connect socket signals
    connect(socket, &QTcpSocket::connected, this, &NetworkClient::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &NetworkClient::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &NetworkClient::onError);

    // Connect reconnect timer
    connect(reconnectTimer, &QTimer::timeout, this, &NetworkClient::attemptReconnect);

    // Connect to server on startup
    connectToServer();
}

NetworkClient::~NetworkClient()
{
    if (socket->state() == QTcpSocket::ConnectedState) {
        socket->disconnectFromHost();
    }
    delete socket;
}

void NetworkClient::connectToServer()
{
    if (socket->state() == QTcpSocket::UnconnectedState) {
        qDebug() << "Attempting to connect to server at" << serverHost << ":" << serverPort;
        socket->connectToHost(serverHost, serverPort);
    }
}

void NetworkClient::sendRequest(const QJsonObject &request)
{
    if (socket->state() == QTcpSocket::ConnectedState) {
        // Convert JSON to bytes
        QJsonDocument doc(request);
        QByteArray data = doc.toJson(QJsonDocument::Compact);

        // Add message delimiter
        data.append('\n');

        // Send data
        socket->write(data);
    } else {
        // Not connected, return error response
        QString errorMsg = "Not connected to server. Attempting to reconnect...";
        emit responseReceived(createErrorResponse(errorMsg));

        // Try to reconnect
        connectToServer();
    }
}

void NetworkClient::disconnect()
{
    if (socket->state() != QTcpSocket::UnconnectedState) {
        socket->disconnectFromHost();
    }
}

bool NetworkClient::isConnected() const
{
    return socket->state() == QTcpSocket::ConnectedState;
}

void NetworkClient::onConnected()
{
    qDebug() << "Connected to server successfully";
    reconnectTimer->stop();
    reconnecting = false;
    emit connected();
}

void NetworkClient::onDisconnected()
{
    qDebug() << "Disconnected from server";
    emit disconnected();

    // Start reconnect timer if not already reconnecting
    if (!reconnecting) {
        reconnecting = true;
        reconnectTimer->start();
    }
}

void NetworkClient::onReadyRead()
{
    // Read all available data
    QByteArray data = socket->readAll();

    // Split data by message delimiter (newline)
    QList<QByteArray> messages = data.split('\n');

    // Process each complete message
    for (const QByteArray &message : messages) {
        if (message.isEmpty()) {
            continue;
        }

        // Parse JSON
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(message, &parseError);

        if (parseError.error == QJsonParseError::NoError) {
            QJsonObject response = doc.object();

            // Check if this is a response or a message
            if (response.contains("action") && response["action"].toString() == "message") {
                emit messageReceived(response);
            } else {
                emit responseReceived(response);
            }
        } else {
            // JSON parsing error
            emit responseReceived(createErrorResponse("Invalid response from server"));
        }
    }
}

void NetworkClient::onError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);

    QString errorMessage = socket->errorString();
    qDebug() << "Socket error:" << errorMessage;
    emit connectionError(errorMessage);

    // Try to reconnect if not already reconnecting
    if (!reconnecting) {
        reconnecting = true;
        reconnectTimer->start();
    }
}

void NetworkClient::attemptReconnect()
{
    if (socket->state() == QTcpSocket::UnconnectedState) {
        qDebug() << "Attempting to reconnect...";
        connectToServer();
    }
}

QJsonObject NetworkClient::createErrorResponse(const QString &message)
{
    QJsonObject response;
    response["status"] = "error";
    response["message"] = message;
    return response;
}
