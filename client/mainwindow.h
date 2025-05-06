#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QSplitter>
#include <QStackedWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QTimer>
#include <QJsonObject>
#include <QDebug>

#include "networkclient.h"
#include "chatbubble.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(int userId, const QString &username, QWidget *parent = nullptr);
    ~MainWindow();
    void loadContacts();

private slots:
    void onSendMessage();
    void onAttachFile();
    void onContactSelected(int row);
    void onNewContactClicked();
    void onSearchTextChanged(const QString &text);
    void onSettingsClicked();
    void onToggleTheme();
    void onLogoutClicked();
    void onNetworkResponse(const QJsonObject &response);
    void onMessageReceived(const QJsonObject &message);

private:
    void setupUI();
    void setupMenuBar();
    void loadChatHistory(int contactId);
    void showWelcomeScreen();
    void filterContacts(const QString &searchText);
    void sendMessage(const QString &content, const QString &type = "text");
    void addMessageToChat(const QJsonObject &message);
    void loadStyleSheet(const QString &path);

    // Network client
    NetworkClient *networkClient;

    // User data
    int currentUserId;
    QString currentUsername;
    int selectedContactId;

    // Main UI components
    QSplitter *mainSplitter;
    QStackedWidget *rightStack;

    // Contacts panel
    QWidget *contactsPanel;
    QListWidget *contactsList;
    QLineEdit *searchEdit;
    QPushButton *newContactButton;
    QPushButton *settingsButton;
    QPushButton *themeButton;

    // Chat panel
    QWidget *chatPanel;
    QWidget *welcomePanel;
    QLabel *chatHeaderLabel;
    QScrollArea *chatScrollArea;
    QWidget *chatContainer;
    QVBoxLayout *chatLayout;
    QLineEdit *messageEdit;
    QPushButton *sendButton;
    QPushButton *attachButton;

    // Status bar labels
    QLabel *statusLabel;
    QLabel *connectionStatusLabel;

    // Theme state
    bool isDarkTheme;
};

#endif // MAINWINDOW_H
