#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QDateTime>

class Message
{
public:
    Message() : id(-1), senderId(-1), receiverId(-1), read(false) {}
    
    int id;
    int senderId;
    int receiverId;
    QString content;
    QString type;
    bool read;
    QDateTime timestamp;
};

#endif // MESSAGE_H