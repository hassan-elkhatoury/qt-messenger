#ifndef USER_H
#define USER_H

#include <QString>
#include <QDateTime>

class User
{
public:
    User() : id(-1) {}
    
    int id;
    QString username;
    QString email;
    QString password;
    QDateTime createdAt;
};

#endif // USER_H