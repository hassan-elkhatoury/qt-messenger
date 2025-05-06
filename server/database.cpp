#include "database.h"
#include <QDir>
#include <QStandardPaths>
#include <QVariant>
#include <QDateTime>

Database::Database(QObject *parent)
    : QObject(parent)
{
}

Database::~Database()
{
    if (db.isOpen()) {
        db.close();
    }
}

bool Database::initialize()
{
    // Set up database directory
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // Initialize database
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dataPath + "/messenger.db");

    if (!db.open()) {
        qCritical() << "Failed to open database:" << db.lastError().text();
        return false;
    }

    return createTables();
}

bool Database::createTables()
{
    QSqlQuery query;

    // Users table
    if (!query.exec("CREATE TABLE IF NOT EXISTS users ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "username TEXT UNIQUE NOT NULL, "
                    "email TEXT UNIQUE NOT NULL, "
                    "password TEXT NOT NULL, "
                    "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
                    ")")) {
        qCritical() << "Failed to create users table:" << query.lastError().text();
        return false;
    }

    // Contacts table
    if (!query.exec("CREATE TABLE IF NOT EXISTS contacts ("
                    "user_id INTEGER NOT NULL, "
                    "contact_id INTEGER NOT NULL, "
                    "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
                    "PRIMARY KEY (user_id, contact_id), "
                    "FOREIGN KEY (user_id) REFERENCES users(id), "
                    "FOREIGN KEY (contact_id) REFERENCES users(id)"
                    ")")) {
        qCritical() << "Failed to create contacts table:" << query.lastError().text();
        return false;
    }

    // Messages table
    if (!query.exec("CREATE TABLE IF NOT EXISTS messages ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "sender_id INTEGER NOT NULL, "
                    "receiver_id INTEGER NOT NULL, "
                    "content TEXT NOT NULL, "
                    "type TEXT DEFAULT 'text', "
                    "read BOOLEAN DEFAULT 0, "
                    "timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
                    "FOREIGN KEY (sender_id) REFERENCES users(id), "
                    "FOREIGN KEY (receiver_id) REFERENCES users(id)"
                    ")")) {
        qCritical() << "Failed to create messages table:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Database::addUser(User &user)
{
    QSqlQuery query;
    query.prepare("INSERT INTO users (username, email, password) "
                  "VALUES (:username, :email, :password)");
    query.bindValue(":username", user.username);
    query.bindValue(":email", user.email);
    query.bindValue(":password", user.password);

    if (!query.exec()) {
        qWarning() << "Failed to add user:" << query.lastError().text();
        return false;
    }

    user.id = query.lastInsertId().toInt();
    return true;
}

bool Database::authenticateUser(const QString &username, const QString &password, User &user)
{
    QSqlQuery query;
    query.prepare("SELECT id, username, email, password FROM users "
                  "WHERE (username = :username OR email = :username) "
                  "AND password = :password");
    query.bindValue(":username", username);
    query.bindValue(":password", password);

    if (!query.exec()) {
        qWarning() << "Authentication query failed:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        user.id = query.value(0).toInt();
        user.username = query.value(1).toString();
        user.email = query.value(2).toString();
        user.password = query.value(3).toString();
        return true;
    }

    return false;
}

bool Database::getUserById(int id, User &user)
{
    QSqlQuery query;
    query.prepare("SELECT id, username, email FROM users WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Get user query failed:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        user.id = query.value(0).toInt();
        user.username = query.value(1).toString();
        user.email = query.value(2).toString();
        return true;
    }

    return false;
}

User Database::getUserById(int id)
{
    User user;
    getUserById(id, user);
    return user;
}

bool Database::getUserByUsername(const QString &username, User &user)
{
    QSqlQuery query;
    query.prepare("SELECT id, username, email FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec()) {
        qWarning() << "Get user by username query failed:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        user.id = query.value(0).toInt();
        user.username = query.value(1).toString();
        user.email = query.value(2).toString();
        return true;
    }

    return false;
}

bool Database::usernameExists(const QString &username)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec()) {
        qWarning() << "Username check query failed:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

bool Database::emailExists(const QString &email)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM users WHERE email = :email");
    query.bindValue(":email", email);

    if (!query.exec()) {
        qWarning() << "Email check query failed:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

QList<User> Database::getContacts(int userId)
{
    QList<User> contacts;
    QSqlQuery query;

    query.prepare(
        "SELECT u.* FROM users u "
        "INNER JOIN contacts c ON u.id = c.contact_id "
        "WHERE c.user_id = :userId "
        "ORDER BY u.username"
        );

    query.bindValue(":userId", userId);

    if (query.exec()) {
        while (query.next()) {
            User contact;
            contact.id = query.value("id").toInt();
            contact.username = query.value("username").toString();
            contact.email = query.value("email").toString();
            contacts.append(contact);
        }
    } else {
        qWarning() << "Failed to get contacts:" << query.lastError().text();
    }

    return contacts;
}

bool Database::addContact(int userId, int contactId)
{
    QSqlQuery query;

    if (!db.transaction()) {
        qWarning() << "Failed to start transaction:" << db.lastError().text();
        return false;
    }

    // Add contact (bidirectional)
    query.prepare("INSERT INTO contacts (user_id, contact_id) VALUES (:userId, :contactId)");
    query.bindValue(":userId", userId);
    query.bindValue(":contactId", contactId);

    if (!query.exec()) {
        db.rollback();
        qWarning() << "Failed to add contact:" << query.lastError().text();
        return false;
    }

    // Add reverse contact
    query.prepare("INSERT INTO contacts (user_id, contact_id) VALUES (:contactId, :userId)");
    query.bindValue(":userId", userId);
    query.bindValue(":contactId", contactId);

    if (!query.exec()) {
        db.rollback();
        qWarning() << "Failed to add reverse contact:" << query.lastError().text();
        return false;
    }

    return db.commit();
}

bool Database::isContactExists(int userId, int contactId)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM contacts WHERE user_id = :userId AND contact_id = :contactId");
    query.bindValue(":userId", userId);
    query.bindValue(":contactId", contactId);

    if (!query.exec()) {
        qWarning() << "Contact check query failed:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

QList<QPair<User, QPair<Message, int>>> Database::getUserContacts(int userId)
{
    QList<QPair<User, QPair<Message, int>>> result;

    QSqlQuery query;
    query.prepare(
        "SELECT u.id, u.username, u.email, "
        "(SELECT m.id FROM messages m "
        "WHERE (m.sender_id = c.user_id AND m.receiver_id = c.contact_id) "
        "OR (m.sender_id = c.contact_id AND m.receiver_id = c.user_id) "
        "ORDER BY m.timestamp DESC LIMIT 1) as last_message_id, "
        "(SELECT COUNT(*) FROM messages m "
        "WHERE m.sender_id = c.contact_id AND m.receiver_id = c.user_id AND m.read = 0) as unread_count "
        "FROM contacts c "
        "JOIN users u ON c.contact_id = u.id "
        "WHERE c.user_id = :userId"
        );
    query.bindValue(":userId", userId);

    if (!query.exec()) {
        qWarning() << "Get contacts query failed:" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        User user;
        user.id = query.value(0).toInt();
        user.username = query.value(1).toString();
        user.email = query.value(2).toString();

        int lastMessageId = query.value(3).toInt();
        int unreadCount = query.value(4).toInt();

        Message lastMessage;
        if (lastMessageId > 0) {
            QSqlQuery msgQuery;
            msgQuery.prepare(
                "SELECT id, sender_id, receiver_id, content, type, read, timestamp "
                "FROM messages WHERE id = :id"
                );
            msgQuery.bindValue(":id", lastMessageId);

            if (msgQuery.exec() && msgQuery.next()) {
                lastMessage.id = msgQuery.value(0).toInt();
                lastMessage.senderId = msgQuery.value(1).toInt();
                lastMessage.receiverId = msgQuery.value(2).toInt();
                lastMessage.content = msgQuery.value(3).toString();
                lastMessage.type = msgQuery.value(4).toString();
                lastMessage.read = msgQuery.value(5).toBool();
                lastMessage.timestamp = msgQuery.value(6).toDateTime();
            }
        }

        result.append(qMakePair(user, qMakePair(lastMessage, unreadCount)));
    }

    return result;
}

bool Database::addMessage(Message &message)
{
    QSqlQuery query;
    query.prepare("INSERT INTO messages (sender_id, receiver_id, content, type, read, timestamp) "
                  "VALUES (:senderId, :receiverId, :content, :type, :read, :timestamp)");
    query.bindValue(":senderId", message.senderId);
    query.bindValue(":receiverId", message.receiverId);
    query.bindValue(":content", message.content);
    query.bindValue(":type", message.type);
    query.bindValue(":read", message.read);
    query.bindValue(":timestamp", message.timestamp);

    if (!query.exec()) {
        qWarning() << "Failed to add message:" << query.lastError().text();
        return false;
    }

    message.id = query.lastInsertId().toInt();
    return true;
}

QList<Message> Database::getChatHistory(int userId, int contactId)
{
    QList<Message> messages;

    QSqlQuery query;
    query.prepare(
        "SELECT id, sender_id, receiver_id, content, type, read, timestamp "
        "FROM messages "
        "WHERE (sender_id = :userId AND receiver_id = :contactId) "
        "OR (sender_id = :contactId AND receiver_id = :userId) "
        "ORDER BY timestamp ASC"
        );
    query.bindValue(":userId", userId);
    query.bindValue(":contactId", contactId);

    if (!query.exec()) {
        qWarning() << "Get chat history query failed:" << query.lastError().text();
        return messages;
    }

    while (query.next()) {
        Message message;
        message.id = query.value(0).toInt();
        message.senderId = query.value(1).toInt();
        message.receiverId = query.value(2).toInt();
        message.content = query.value(3).toString();
        message.type = query.value(4).toString();
        message.read = query.value(5).toBool();
        message.timestamp = query.value(6).toDateTime();

        messages.append(message);
    }

    return messages;
}

bool Database::markMessagesAsRead(int senderId, int receiverId)
{
    QSqlQuery query;
    query.prepare(
        "UPDATE messages SET read = 1 "
        "WHERE sender_id = :senderId AND receiver_id = :receiverId AND read = 0"
        );
    query.bindValue(":senderId", senderId);
    query.bindValue(":receiverId", receiverId);

    if (!query.exec()) {
        qWarning() << "Mark messages as read query failed:" << query.lastError().text();
        return false;
    }

    return true;
}

int Database::getUnreadMessageCount(int userId, int contactId)
{
    QSqlQuery query;
    query.prepare(
        "SELECT COUNT(*) FROM messages "
        "WHERE sender_id = :contactId "
        "AND receiver_id = :userId "
        "AND read = 0"
        );
    query.bindValue(":userId", userId);
    query.bindValue(":contactId", contactId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}
