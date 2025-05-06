#include "contactlistitem.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QPixmap>
#include <QPainterPath>
#include <QMouseEvent>

ContactListItem::ContactListItem(int id, const QString &name, const QString &lastMessage, 
                               const QString &lastMessageTime, int unreadCount, 
                               QWidget *parent)
    : QWidget(parent)
    , id(id)
    , name(name)
    , lastMessage(lastMessage)
    , lastMessageTime(lastMessageTime)
    , unreadCount(unreadCount)
{
    setupUI();
}

void ContactListItem::setupUI()
{
    setMinimumHeight(60);
    
    // Main layout
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(10);
    
    // Avatar
    avatarLabel = new QLabel();
    avatarLabel->setFixedSize(40, 40);
    
    // Create a colored circle with the first letter of the name
    QPixmap avatar(40, 40);
    avatar.fill(Qt::transparent);
    
    QPainter painter(&avatar);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // Draw circle background
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 150, 136)); // Teal color
    painter.drawEllipse(0, 0, 40, 40);
    
    // Draw text
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(16);
    font.setBold(true);
    painter.setFont(font);
    
    QString firstLetter = name.isEmpty() ? QChar('?') : name.at(0).toUpper();
    painter.drawText(0, 0, 40, 40, Qt::AlignCenter, firstLetter);
    
    avatarLabel->setPixmap(avatar);
    
    // Contact info layout
    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->setSpacing(2);
    
    // Name and time layout
    QHBoxLayout *nameTimeLayout = new QHBoxLayout();
    nameTimeLayout->setContentsMargins(0, 0, 0, 0);
    
    nameLabel = new QLabel(name);
    QFont nameFont = nameLabel->font();
    nameFont.setBold(true);
    nameLabel->setFont(nameFont);
    
    timeLabel = new QLabel(lastMessageTime);
    QFont timeFont = timeLabel->font();
    timeFont.setPointSize(timeFont.pointSize() - 1);
    timeLabel->setFont(timeFont);
    timeLabel->setAlignment(Qt::AlignRight);
    
    nameTimeLayout->addWidget(nameLabel, 1);
    nameTimeLayout->addWidget(timeLabel);
    
    // Message and unread count layout
    QHBoxLayout *messageCountLayout = new QHBoxLayout();
    messageCountLayout->setContentsMargins(0, 0, 0, 0);
    
    messageLabel = new QLabel(lastMessage);
    QFont messageFont = messageLabel->font();
    messageFont.setPointSize(messageFont.pointSize() - 1);
    messageLabel->setFont(messageFont);
    messageLabel->setMaximumWidth(200);
    QFontMetrics metrics(messageLabel->font());
    QString elidedText = metrics.elidedText(lastMessage, Qt::ElideRight, messageLabel->width());
    messageLabel->setText(elidedText);
    
    unreadLabel = new QLabel();
    unreadLabel->setFixedSize(20, 20);
    unreadLabel->setAlignment(Qt::AlignCenter);
    unreadLabel->setStyleSheet("background-color: #25D366; color: white; border-radius: 10px;");
    
    if (unreadCount > 0) {
        unreadLabel->setText(QString::number(unreadCount));
        unreadLabel->setVisible(true);
    } else {
        unreadLabel->setVisible(false);
    }
    
    messageCountLayout->addWidget(messageLabel, 1);
    messageCountLayout->addWidget(unreadLabel);
    
    // Add layouts to info layout
    infoLayout->addLayout(nameTimeLayout);
    infoLayout->addLayout(messageCountLayout);
    
    // Add widgets to main layout
    mainLayout->addWidget(avatarLabel);
    mainLayout->addLayout(infoLayout, 1);
    
    // Set cursor
    setCursor(Qt::PointingHandCursor);
}

void ContactListItem::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ContactListItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        resetUnreadCount();
    }
    QWidget::mousePressEvent(event);
}

void ContactListItem::updateLastMessage(const QString &message, const QString &timestamp)
{
    lastMessage = message;
    lastMessageTime = timestamp;
    
    messageLabel->setText(message);
    timeLabel->setText(timestamp);
}

void ContactListItem::incrementUnreadCount()
{
    unreadCount++;
    unreadLabel->setText(QString::number(unreadCount));
    unreadLabel->setVisible(true);
}

void ContactListItem::resetUnreadCount()
{
    unreadCount = 0;
    unreadLabel->setVisible(false);
}
