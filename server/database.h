// server/database.h
#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "user.h"
#include "message.h"

class Database : public QObject
{
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);
    ~Database();

    bool initialize();

    // User management
    bool addUser(User &user);
    User getUserById(int id);
    bool authenticateUser(const QString &username, const QString &password, User &user);
    bool getUserById(int id, User &user);
    QList<QPair<User, QPair<Message, int>>> getUserContacts(int userId);
    bool getUserByUsername(const QString &username, User &user);
    bool usernameExists(const QString &username);
    bool emailExists(const QString &email);

    // Contact management
    bool addContact(int userId, int contactId);
    bool removeContact(int userId, int contactId);
    QList<User> getContacts(int userId);
    bool isContactExists(int userId, int contactId);

    // Message management
    bool addMessage(Message &message);
    QList<Message> getChatHistory(int userId, int contactId);
    bool markMessagesAsRead(int senderId, int receiverId);
    int getUnreadMessageCount(int userId, int contactId);

private:
    bool createTables();
    QSqlDatabase db;
};

#endif // DATABASE_H
