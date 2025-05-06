#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "networkclient.h"

class RegisterWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit RegisterWindow(QWidget *parent = nullptr);
    ~RegisterWindow();

private slots:
    void onRegisterClicked();
    void onBackToLoginClicked();
    void onNetworkResponse(const QJsonObject &response);

private:
    void setupUI();

    QLineEdit *usernameEdit;
    QLineEdit *emailEdit;
    QLineEdit *passwordEdit;
    QLineEdit *confirmPasswordEdit;
    QPushButton *registerButton;
    QPushButton *backButton;
    QLabel *statusLabel;
    
    NetworkClient *networkClient;
};

#endif // REGISTERWINDOW_H