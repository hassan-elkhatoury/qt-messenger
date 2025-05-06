#include "server.h"
#include "user.h"
#include "message.h"

#include <QHostAddress>
#include <QJsonArray>
#include <QDateTime>

Server::Server(quint16 port, Database *database, QObject *parent)
    : QObject(parent)
    , server(new QTcpServer(this))
    , database(database)
{
    // Configure server
    server->setMaxPendingConnections(100); // Limit concurrent connections

    // Set port
    if (!server->listen(QHostAddress::Any, port)) {
        qCritical() << "Server failed to start:" << server->errorString();
    }

    // Connect signals
    connect(server, &QTcpServer::newConnection, this, &Server::onNewConnection);
}

Server::~Server()
{
    stop();
}

bool Server::start()
{
    if (server->isListening()) {
        return true;
    }

    return server->listen(QHostAddress::Any, server->serverPort());
}

void Server::stop()
{
    if (!server->isListening())
        return;

    for (QTcpSocket *client : socketUsers.keys()) {
        if (client->state() == QAbstractSocket::ConnectedState)
            client->disconnectFromHost();
        client->deleteLater();
    }

    socketUsers.clear();
    userConnections.clear();

    server->close();
}

void Server::onNewConnection()
{
    QTcpSocket *client = server->nextPendingConnection();

    // Connect socket signals
    connect(client, &QTcpSocket::readyRead, this, &Server::onReadyRead);
    connect(client, &QTcpSocket::disconnected, this, &Server::onClientDisconnected);

    qInfo() << "New client connected:" << client->peerAddress().toString();
}

void Server::onClientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) {
        return;
    }

    // Remove from user maps
    if (socketUsers.contains(client)) {
        int userId = socketUsers[client];
        userConnections.remove(userId);
        socketUsers.remove(client);

        qInfo() << "Client disconnected, user ID:" << userId;
    }

    client->deleteLater();
}

void Server::onReadyRead()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    while (client->canReadLine()) {
        QByteArray line = client->readLine().trimmed();
        if (line.isEmpty()) continue;

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(line, &parseError);

        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            handleRequest(client, doc.object());
        } else {
            QJsonObject errorResponse;
            errorResponse["status"] = "error";
            errorResponse["message"] = "Invalid JSON: " + parseError.errorString();
            sendResponse(client, errorResponse);
        }
    }
}

void Server::handleRequest(QTcpSocket *client, const QJsonObject &request)
{
    QString action = request["action"].toString();

    qInfo() << "Received request:" << action;

    if (action == "login") {
        handleLogin(client, request);
    }
    else if (action == "register") {
        handleRegister(client, request);
    }
    else if (action == "getContacts") {
        handleGetContacts(client, request);
    }
    else if (action == "getChatHistory") {
        handleGetChatHistory(client, request);
    }
    else if (action == "sendMessage") {
        handleSendMessage(client, request);
    }
    else if (action == "addContact") {
        handleAddContact(client, request);
    }
    else {
        // Unknown action
        QJsonObject errorResponse;
        errorResponse["status"] = "error";
        errorResponse["message"] = "Unknown action: " + action;
        sendResponse(client, errorResponse);
    }
}

void Server::handleLogin(QTcpSocket *client, const QJsonObject &request)
{
    QString username = request["username"].toString();
    QString password = request["password"].toString();

    QJsonObject response;

    // Authenticate user
    User user;
    bool success = database->authenticateUser(username, password, user);

    if (success) {
        // Login successful
        response["status"] = "success";

        // Add user data
        QJsonObject userData;
        userData["id"] = user.id;
        userData["username"] = user.username;
        userData["email"] = user.email;

        response["user"] = userData;

        // Associate socket with user
        userConnections[user.id] = client;
        socketUsers[client] = user.id;

        qInfo() << "User logged in:" << username << "(ID:" << user.id << ")";
    } else {
        // Login failed
        response["status"] = "error";
        response["message"] = "Invalid username or password";

        qInfo() << "Login failed for username:" << username;
    }

    // Send response
    sendResponse(client, response);
}

void Server::handleRegister(QTcpSocket *client, const QJsonObject &request)
{
    QString username = request["username"].toString();
    QString email = request["email"].toString();
    QString password = request["password"].toString();

    QJsonObject response;

    // Check if username already exists
    if (database->usernameExists(username)) {
        response["status"] = "error";
        response["message"] = "Username already exists";
    }
    // Check if email already exists
    else if (database->emailExists(email)) {
        response["status"] = "error";
        response["message"] = "Email already in use";
    }
    else {
        // Create new user
        User user;
        user.username = username;
        user.email = email;
        user.password = password;

        bool success = database->addUser(user);

        if (success) {
            response["status"] = "success";
            response["message"] = "User registered successfully";

            qInfo() << "New user registered:" << username;
        } else {
            response["status"] = "error";
            response["message"] = "Failed to register user";

            qWarning() << "Failed to register user:" << username;
        }
    }

    // Send response
    sendResponse(client, response);
}

void Server::handleGetContacts(QTcpSocket *client, const QJsonObject &request)
{
    int userId = request["userId"].toInt();

    QJsonObject response;
    response["action"] = "getContacts";

    // Get contacts with their last messages and unread counts
    QList<QPair<User, QPair<Message, int>>> contacts = database->getUserContacts(userId);

    QJsonArray contactsArray;
    for (const auto &contact : contacts) {
        QJsonObject contactObj;
        contactObj["id"] = contact.first.id;
        contactObj["username"] = contact.first.username;

        // Add last message info if available
        if (contact.second.first.id > 0) {
            contactObj["lastMessage"] = contact.second.first.content;
            contactObj["lastMessageTime"] = contact.second.first.timestamp.toString(Qt::ISODate);
        } else {
            contactObj["lastMessage"] = "";
            contactObj["lastMessageTime"] = "";
        }

        // Add unread count
        contactObj["unreadCount"] = contact.second.second;

        contactsArray.append(contactObj);
    }

    response["status"] = "success";
    response["contacts"] = contactsArray;

    // Send response
    sendResponse(client, response);
}

void Server::handleGetChatHistory(QTcpSocket *client, const QJsonObject &request)
{
    int userId = request["userId"].toInt();
    int contactId = request["contactId"].toInt();

    QJsonObject response;
    response["action"] = "getChatHistory";

    // Get chat history
    QList<Message> messages = database->getChatHistory(userId, contactId);

    QJsonArray messagesArray;
    for (const Message &message : messages) {
        QJsonObject messageObj;
        messageObj["id"] = message.id;
        messageObj["senderId"] = message.senderId;
        messageObj["receiverId"] = message.receiverId;
        messageObj["content"] = message.content;
        messageObj["timestamp"] = message.timestamp.toString(Qt::ISODate);
        messageObj["type"] = message.type;

        // Get sender name
        User sender = database->getUserById(message.senderId);
        messageObj["senderName"] = sender.username;

        messagesArray.append(messageObj);
    }

    response["status"] = "success";
    response["messages"] = messagesArray;

    // Mark messages as read
    database->markMessagesAsRead(contactId, userId);

    // Send response
    sendResponse(client, response);
}

void Server::handleSendMessage(QTcpSocket *client, const QJsonObject &request)
{
    int senderId = request["senderId"].toInt();
    int receiverId = request["receiverId"].toInt();
    QString content = request["content"].toString();
    QString type = request.contains("type") ? request["type"].toString() : "text";
    QString timestampStr = request["timestamp"].toString();

    QDateTime timestamp = QDateTime::fromString(timestampStr, Qt::ISODate);
    if (!timestamp.isValid()) {
        timestamp = QDateTime::currentDateTime();
    }

    // Create message
    Message message;
    message.senderId = senderId;
    message.receiverId = receiverId;
    message.content = content;
    message.timestamp = timestamp;
    message.type = type;
    message.read = false;

    // Save message to database
    bool success = database->addMessage(message);

    QJsonObject response;
    response["action"] = "sendMessage";

    if (success) {
        response["status"] = "success";
        response["messageId"] = message.id;

        // Send message to receiver if online
        if (userConnections.contains(receiverId)) {
            QJsonObject messageObj = request;
            messageObj["action"] = "message";
            messageObj["id"] = message.id;

            broadcastToUser(receiverId, messageObj);
        }

        qInfo() << "Message sent from" << senderId << "to" << receiverId;
    } else {
        response["status"] = "error";
        response["message"] = "Failed to send message";

        qWarning() << "Failed to send message from" << senderId << "to" << receiverId;
    }

    // Send response
    sendResponse(client, response);
}

void Server::handleAddContact(QTcpSocket *client, const QJsonObject &request)
{
    int userId = request["userId"].toInt();
    QString contactUsername = request["contactUsername"].toString();

    QJsonObject response;
    response["action"] = "addContact";

    // Get contact by username
    User contactUser;
    bool found = database->getUserByUsername(contactUsername, contactUser);

    if (!found) {
        response["status"] = "error";
        response["message"] = "User not found";
    }
    else if (contactUser.id == userId) {
        response["status"] = "error";
        response["message"] = "Cannot add yourself as contact";
    }
    else {
        // Check if already a contact
        if (database->isContactExists(userId, contactUser.id)) {
            response["status"] = "error";
            response["message"] = "User is already in your contacts";
        } else {
            // Add contact
            bool success = database->addContact(userId, contactUser.id);

            if (success) {
                response["status"] = "success";
                response["message"] = "Contact added successfully";

                qInfo() << "Contact added: User" << userId << "added" << contactUser.id;
            } else {
                response["status"] = "error";
                response["message"] = "Failed to add contact";

                qWarning() << "Failed to add contact: User" << userId << "tried to add" << contactUser.id;
            }
        }
    }

    // Send response
    sendResponse(client, response);
}

void Server::sendResponse(QTcpSocket *client, const QJsonObject &response)
{
    // Convert JSON to bytes
    QJsonDocument doc(response);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    // Add message delimiter
    data.append('\n');

    // Send data
    client->write(data);
}

void Server::broadcastToUser(int userId, const QJsonObject &message)
{
    if (userConnections.contains(userId)) {
        QTcpSocket *client = userConnections[userId];
        sendResponse(client, message);
    }
}
