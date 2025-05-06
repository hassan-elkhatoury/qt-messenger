#ifndef CHATBUBBLE_H
#define CHATBUBBLE_H

#include <QWidget>
#include <QLabel>

class ChatBubble : public QWidget
{
    Q_OBJECT

public:
    explicit ChatBubble(const QString &message, const QString &timestamp, 
                        bool fromMe, const QString &type = "text", 
                        QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUI();
    QPixmap createRoundedPixmap(const QPixmap &source, int radius);

    QString message;
    QString timestamp;
    bool fromMe;
    QString type;
    
    QLabel *messageLabel;
    QLabel *timestampLabel;
    QLabel *fileIconLabel;
};

#endif // CHATBUBBLE_H