#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include "networkclient.h"

class LoginWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onToggleTheme();
    void onNetworkResponse(const QJsonObject &response);

private:
    void setupUI();
    void loadStyleSheet(const QString &path);

    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    QPushButton *registerButton;
    QPushButton *themeButton;
    QLabel *statusLabel;
    QCheckBox *rememberMeCheckBox;
    NetworkClient *networkClient;
    bool isDarkTheme;
    

};

#endif // LOGINWINDOW_H
