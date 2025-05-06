#ifndef CONTACTLISTITEM_H
#define CONTACTLISTITEM_H

#include <QWidget>
#include <QLabel>

class ContactListItem : public QWidget
{
    Q_OBJECT

public:
    explicit ContactListItem(int id, const QString &name, const QString &lastMessage, 
                            const QString &lastMessageTime, int unreadCount, 
                            QWidget *parent = nullptr);
    
    int contactId() const { return id; }
    QString contactName() const { return name; }
    
    void updateLastMessage(const QString &message, const QString &timestamp);
    void incrementUnreadCount();
    void resetUnreadCount();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void setupUI();
    
    int id;
    QString name;
    QString lastMessage;
    QString lastMessageTime;
    int unreadCount;
    
    QLabel *avatarLabel;
    QLabel *nameLabel;
    QLabel *messageLabel;
    QLabel *timeLabel;
    QLabel *unreadLabel;
};

#endif // CONTACTLISTITEM_H