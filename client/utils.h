#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QDateTime>
#include <QCryptographicHash>

class Utils
{
public:
    // Hash a password using SHA-256
    static QString hashPassword(const QString &password) {
        QByteArray hash = QCryptographicHash::hash(
            password.toUtf8(), 
            QCryptographicHash::Sha256
        ).toHex();
        return QString(hash);
    }
    
    // Format timestamp for display
    static QString formatTimestamp(const QDateTime &timestamp) {
        QDateTime now = QDateTime::currentDateTime();
        if (timestamp.date() == now.date()) {
            // Today, show only time
            return timestamp.toString("hh:mm AP");
        } else if (timestamp.daysTo(now) <= 7) {
            // Within a week, show day name and time
            return timestamp.toString("ddd hh:mm AP");
        } else {
            // Older, show date
            return timestamp.toString("yyyy-MM-dd");
        }
    }
};

#endif // UTILS_H