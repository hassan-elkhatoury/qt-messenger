#include "chatbubble.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QDateTime>
#include <QStyle>
#include <QStyleOption>

ChatBubble::ChatBubble(const QString &message, const QString &timestamp, 
                       bool fromMe, const QString &type, QWidget *parent)
    : QWidget(parent)
    , message(message)
    , timestamp(timestamp)
    , fromMe(fromMe)
    , type(type)
{
    setupUI();
}

void ChatBubble::setupUI()
{
    // Main layout
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Add spacer if message is from me (align right)
    if (fromMe) {
        mainLayout->addStretch();
    }
    
    // Create bubble container
    QWidget *bubbleWidget = new QWidget(this);
    bubbleWidget->setMaximumWidth(400);
    
    // Bubble layout
    QVBoxLayout *bubbleLayout = new QVBoxLayout(bubbleWidget);
    bubbleLayout->setContentsMargins(12, 8, 12, 8);
    bubbleLayout->setSpacing(4);
    
    // Message label
    messageLabel = new QLabel(message);
    messageLabel->setWordWrap(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    
    // Check if this is a file message
    if (type == "file") {
        // File message layout
        QHBoxLayout *fileLayout = new QHBoxLayout();
        
        // File icon
        fileIconLabel = new QLabel();
        QPixmap fileIcon(":/resources/icons/attachment.png");
        fileIconLabel->setPixmap(fileIcon.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        
        fileLayout->addWidget(fileIconLabel);
        fileLayout->addWidget(messageLabel);
        
        bubbleLayout->addLayout(fileLayout);
    } else {
        bubbleLayout->addWidget(messageLabel);
    }
    
    // Timestamp label
    timestampLabel = new QLabel(timestamp);
    QFont timestampFont = timestampLabel->font();
    timestampFont.setPointSize(timestampFont.pointSize() - 2);
    timestampLabel->setFont(timestampFont);
    timestampLabel->setAlignment(fromMe ? Qt::AlignRight : Qt::AlignLeft);
    
    bubbleLayout->addWidget(timestampLabel);
    
    mainLayout->addWidget(bubbleWidget);
    
    // Add spacer if message is from someone else (align left)
    if (!fromMe) {
        mainLayout->addStretch();
    }
    
    // Set object name for style
    if (fromMe) {
        bubbleWidget->setObjectName("myBubble");
    } else {
        bubbleWidget->setObjectName("theirBubble");
    }
}

void ChatBubble::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

QPixmap ChatBubble::createRoundedPixmap(const QPixmap &source, int radius)
{
    QPixmap result(source.size());
    result.fill(Qt::transparent);
    
    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    QPainterPath path;
    path.addRoundedRect(QRectF(0, 0, source.width(), source.height()), radius, radius);
    
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, source);
    
    return result;
}