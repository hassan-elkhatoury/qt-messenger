#include "mainwindow.h"
#include "loginwindow.h"
#include "contactlistitem.h"
#include "utils.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFile>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDateTime>
#include <QScrollBar>
#include <QIcon>
#include <QInputDialog>
#include <QStatusBar>
#include <QApplication>


MainWindow::MainWindow(int userId, const QString &username, QWidget *parent)
    : QMainWindow(parent)
    , networkClient(new NetworkClient(this))
    , currentUserId(userId)
    , currentUsername(username)
    , selectedContactId(-1)
    , isDarkTheme(false)
{
    setupUI();
    setupMenuBar();

    // Connect network signals
    connect(networkClient, &NetworkClient::responseReceived,
            this, &MainWindow::onNetworkResponse);
    connect(networkClient, &NetworkClient::messageReceived,
            this, &MainWindow::onMessageReceived);
    connect(networkClient, &NetworkClient::connected, [this]() {
        connectionStatusLabel->setText("Connected");
        connectionStatusLabel->setStyleSheet("color: green;");

        // Load contacts when connected to server
        QTimer::singleShot(500, this, &MainWindow::loadContacts);
    });
    connect(networkClient, &NetworkClient::disconnected, [this]() {
        connectionStatusLabel->setText("Disconnected");
        connectionStatusLabel->setStyleSheet("color: red;");
    });

    // Start with welcome screen
    showWelcomeScreen();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    // Configure main window
    setWindowTitle("Qt Messenger");
    setMinimumSize(900, 600);
    setWindowIcon(QIcon(":/resources/icons/app_logo.png"));

    // Create central widget
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Create main splitter
    mainSplitter = new QSplitter(Qt::Horizontal, centralWidget);

    // Create left panel (contacts)
    contactsPanel = new QWidget();
    contactsPanel->setMinimumWidth(250);
    contactsPanel->setMaximumWidth(350);

    QVBoxLayout *contactsPanelLayout = new QVBoxLayout(contactsPanel);
    contactsPanelLayout->setContentsMargins(10, 10, 10, 10);
    contactsPanelLayout->setSpacing(10);

    // User profile section
    QHBoxLayout *profileLayout = new QHBoxLayout();

    QLabel *profileImageLabel = new QLabel();
    QPixmap profileImage(":/resources/icons/profile.png");
    profileImageLabel->setPixmap(profileImage.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QLabel *usernameLabel = new QLabel(currentUsername);
    QFont usernameFont = usernameLabel->font();
    usernameFont.setBold(true);
    usernameFont.setPointSize(12);
    usernameLabel->setFont(usernameFont);

    settingsButton = new QPushButton();
    settingsButton->setIcon(QIcon(":/resources/icons/settings.png"));
    settingsButton->setFlat(true);
    settingsButton->setCursor(Qt::PointingHandCursor);
    settingsButton->setToolTip("Settings");

    profileLayout->addWidget(profileImageLabel);
    profileLayout->addWidget(usernameLabel, 1);
    profileLayout->addWidget(settingsButton);

    // Search box
    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Search contacts...");
    searchEdit->setClearButtonEnabled(true);

    // Contacts list
    contactsList = new QListWidget();
    contactsList->setFrameShape(QFrame::NoFrame);
    contactsList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    contactsList->setSelectionMode(QAbstractItemView::SingleSelection);

    // New contact button
    newContactButton = new QPushButton("New Contact");
    newContactButton->setIcon(QIcon(":/resources/icons/profile.png"));
    newContactButton->setCursor(Qt::PointingHandCursor);

    // Theme button
    themeButton = new QPushButton();
    themeButton->setIcon(QIcon(":/resources/icons/theme.png"));
    themeButton->setFlat(true);
    themeButton->setCursor(Qt::PointingHandCursor);
    themeButton->setToolTip("Toggle Dark/Light Theme");

    // Add widgets to contacts panel
    contactsPanelLayout->addLayout(profileLayout);
    contactsPanelLayout->addWidget(searchEdit);
    contactsPanelLayout->addWidget(contactsList, 1);
    contactsPanelLayout->addWidget(newContactButton);

    // Bottom buttons
    QHBoxLayout *bottomButtonsLayout = new QHBoxLayout();
    bottomButtonsLayout->addWidget(themeButton);

    QPushButton *logoutButton = new QPushButton();
    logoutButton->setIcon(QIcon(":/resources/icons/back.png"));
    logoutButton->setFlat(true);
    logoutButton->setCursor(Qt::PointingHandCursor);
    logoutButton->setToolTip("Logout");

    bottomButtonsLayout->addStretch();
    bottomButtonsLayout->addWidget(logoutButton);

    contactsPanelLayout->addLayout(bottomButtonsLayout);

    // Create right panel with stacked widget (chat and welcome)
    rightStack = new QStackedWidget();

    // Chat panel
    chatPanel = new QWidget();
    QVBoxLayout *chatPanelLayout = new QVBoxLayout(chatPanel);
    chatPanelLayout->setContentsMargins(0, 0, 0, 0);
    chatPanelLayout->setSpacing(0);

    // Chat header
    QWidget *chatHeaderWidget = new QWidget();
    chatHeaderWidget->setMinimumHeight(60);
    QHBoxLayout *chatHeaderLayout = new QHBoxLayout(chatHeaderWidget);

    chatHeaderLabel = new QLabel("Select a contact");
    QFont chatHeaderFont = chatHeaderLabel->font();
    chatHeaderFont.setBold(true);
    chatHeaderFont.setPointSize(12);
    chatHeaderLabel->setFont(chatHeaderFont);

    chatHeaderLayout->addWidget(chatHeaderLabel);

    // Chat area
    chatScrollArea = new QScrollArea();
    chatScrollArea->setWidgetResizable(true);
    chatScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    chatScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    chatScrollArea->setFrameShape(QFrame::NoFrame);

    chatContainer = new QWidget();
    chatLayout = new QVBoxLayout(chatContainer);
    chatLayout->setAlignment(Qt::AlignTop);
    chatLayout->setContentsMargins(15, 15, 15, 15);
    chatLayout->setSpacing(15);

    chatScrollArea->setWidget(chatContainer);

    // Message input area
    QWidget *messageInputWidget = new QWidget();
    messageInputWidget->setMinimumHeight(60);
    QHBoxLayout *messageInputLayout = new QHBoxLayout(messageInputWidget);
    messageInputLayout->setContentsMargins(15, 10, 15, 10);

    attachButton = new QPushButton();
    attachButton->setIcon(QIcon(":/resources/icons/attachment.png"));
    attachButton->setToolTip("Attach File");
    attachButton->setCursor(Qt::PointingHandCursor);

    messageEdit = new QLineEdit();
    messageEdit->setPlaceholderText("Type a message...");

    sendButton = new QPushButton();
    sendButton->setIcon(QIcon(":/resources/icons/send.png"));
    sendButton->setToolTip("Send Message");
    sendButton->setCursor(Qt::PointingHandCursor);

    messageInputLayout->addWidget(attachButton);
    messageInputLayout->addWidget(messageEdit, 1);
    messageInputLayout->addWidget(sendButton);

    // Add widgets to chat panel
    chatPanelLayout->addWidget(chatHeaderWidget);
    chatPanelLayout->addWidget(new QFrame()); // Separator line
    chatPanelLayout->addWidget(chatScrollArea, 1);
    chatPanelLayout->addWidget(new QFrame()); // Separator line
    chatPanelLayout->addWidget(messageInputWidget);

    // Welcome panel (shown when no chat is selected)
    welcomePanel = new QWidget();
    QVBoxLayout *welcomeLayout = new QVBoxLayout(welcomePanel);
    welcomeLayout->setAlignment(Qt::AlignCenter);

    QLabel *welcomeImageLabel = new QLabel();
    QPixmap welcomeImage(":/resources/icons/app_logo.png");
    welcomeImageLabel->setPixmap(welcomeImage.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    welcomeImageLabel->setAlignment(Qt::AlignCenter);

    QLabel *welcomeTextLabel = new QLabel("Select a contact to start chatting");
    QFont welcomeFont = welcomeTextLabel->font();
    welcomeFont.setPointSize(16);
    welcomeTextLabel->setFont(welcomeFont);
    welcomeTextLabel->setAlignment(Qt::AlignCenter);

    welcomeLayout->addWidget(welcomeImageLabel);
    welcomeLayout->addWidget(welcomeTextLabel);

    // Add panels to stacked widget
    rightStack->addWidget(welcomePanel);
    rightStack->addWidget(chatPanel);

    // Add panels to splitter
    mainSplitter->addWidget(contactsPanel);
    mainSplitter->addWidget(rightStack);

    // Set initial splitter sizes
    QList<int> sizes;
    sizes << 300 << 600;
    mainSplitter->setSizes(sizes);

    // Add splitter to main layout
    mainLayout->addWidget(mainSplitter);

    setCentralWidget(centralWidget);

    // Create status bar
    QStatusBar *statusBar = new QStatusBar();
    setStatusBar(statusBar);

    statusLabel = new QLabel("Ready");
    connectionStatusLabel = new QLabel("Connecting...");

    statusBar->addWidget(statusLabel, 1);
    statusBar->addPermanentWidget(connectionStatusLabel);

    // Connect signals
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::onSendMessage);
    connect(messageEdit, &QLineEdit::returnPressed, this, &MainWindow::onSendMessage);
    connect(attachButton, &QPushButton::clicked, this, &MainWindow::onAttachFile);
    connect(contactsList, &QListWidget::currentRowChanged, this, &MainWindow::onContactSelected);
    connect(newContactButton, &QPushButton::clicked, this, &MainWindow::onNewContactClicked);
    connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);
    connect(themeButton, &QPushButton::clicked, this, &MainWindow::onToggleTheme);
    connect(logoutButton, &QPushButton::clicked, this, &MainWindow::onLogoutClicked);
}

void MainWindow::setupMenuBar()
{
    QMenuBar *menuBar = new QMenuBar();

    // File menu
    QMenu *fileMenu = menuBar->addMenu("File");

    QAction *settingsAction = fileMenu->addAction("Settings");
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onSettingsClicked);

    fileMenu->addSeparator();

    QAction *logoutAction = fileMenu->addAction("Logout");
    connect(logoutAction, &QAction::triggered, this, &MainWindow::onLogoutClicked);

    QAction *exitAction = fileMenu->addAction("Exit");
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    // Contacts menu
    QMenu *contactsMenu = menuBar->addMenu("Contacts");

    QAction *newContactAction = contactsMenu->addAction("Add New Contact");
    connect(newContactAction, &QAction::triggered, this, &MainWindow::onNewContactClicked);

    QAction *refreshContactsAction = contactsMenu->addAction("Refresh Contacts");
    connect(refreshContactsAction, &QAction::triggered, this, &MainWindow::loadContacts);

    // View menu
    QMenu *viewMenu = menuBar->addMenu("View");

    QAction *toggleThemeAction = viewMenu->addAction("Toggle Theme");
    connect(toggleThemeAction, &QAction::triggered, this, &MainWindow::onToggleTheme);

    // Help menu
    QMenu *helpMenu = menuBar->addMenu("Help");

    QAction *aboutAction = helpMenu->addAction("About");
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "About Qt Messenger",
                           "Qt Messenger v1.0\n\n"
                           "A modern messaging application built with Qt/C++.\n\n"
                           "© 2025 Qt Messenger Team");
    });

    setMenuBar(menuBar);
}

void MainWindow::loadContacts()
{
    // Clear current list
    contactsList->clear();

    // Request contacts from server
    QJsonObject request;
    request["action"] = "getContacts";
    request["userId"] = currentUserId;

    networkClient->sendRequest(request);
    statusLabel->setText("Loading contacts...");
}


void MainWindow::onSendMessage()
{
    QString content = messageEdit->text().trimmed();
    if (content.isEmpty() || selectedContactId == -1) {
        return;
    }

    sendMessage(content);
    messageEdit->clear();
}

void MainWindow::sendMessage(const QString &content, const QString &type)
{
    // Prepare message data
    QJsonObject request;
    request["action"] = "sendMessage";
    request["senderId"] = currentUserId;
    request["senderName"] = currentUsername;
    request["receiverId"] = selectedContactId;
    request["content"] = content;
    request["type"] = type;
    request["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    // Send to server
    networkClient->sendRequest(request);

    // Add message to local chat (optimistic UI update)
    addMessageToChat(request);
}

void MainWindow::onAttachFile()
{
    if (selectedContactId == -1) {
        QMessageBox::warning(this, "Warning", "Please select a contact first.");
        return;
    }

    QString filePath = QFileDialog::getOpenFileName(this, "Select File to Send");
    if (filePath.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();

    // For this demo, we just send the file name
    // In a real app, you'd upload the file and send a reference
    QString message = "File: " + fileName;
    sendMessage(message, "file");
}

void MainWindow::onContactSelected(int row)
{
    if (row < 0) {
        selectedContactId = -1;
        return;
    }

    // Get contact data from item
    QListWidgetItem *item = contactsList->item(row);
    ContactListItem *contactItem = dynamic_cast<ContactListItem*>(
        contactsList->itemWidget(item));

    if (contactItem) {
        selectedContactId = contactItem->contactId();
        chatHeaderLabel->setText(contactItem->contactName());

        // Clear any existing messages in the chat view
        while (QLayoutItem *item = chatLayout->takeAt(0)) {
            delete item->widget();
            delete item;
        }

        // Show "Loading messages..." indicator
        QLabel *loadingLabel = new QLabel("Loading messages...");
        loadingLabel->setAlignment(Qt::AlignCenter);
        chatLayout->addWidget(loadingLabel);

        // Load chat history
        loadChatHistory(selectedContactId);

        // Show chat panel
        rightStack->setCurrentWidget(chatPanel);

        // Reset unread count for this contact
        contactItem->resetUnreadCount();
    }
}

void MainWindow::onNewContactClicked()
{
    QString username = QInputDialog::getText(this, "Add Contact",
                                             "Enter username to add:");
    if (username.isEmpty()) {
        return;
    }

    // Send add contact request
    QJsonObject request;
    request["action"] = "addContact";
    request["userId"] = currentUserId;
    request["contactUsername"] = username;

    networkClient->sendRequest(request);
}

void MainWindow::onSearchTextChanged(const QString &text)
{
    filterContacts(text);
}

void MainWindow::filterContacts(const QString &searchText)
{
    // Show/hide contacts based on search text
    for (int i = 0; i < contactsList->count(); ++i) {
        QListWidgetItem *item = contactsList->item(i);
        ContactListItem *contactItem = dynamic_cast<ContactListItem*>(
            contactsList->itemWidget(item));

        if (contactItem) {
            bool visible = contactItem->contactName().contains(searchText, Qt::CaseInsensitive);
            item->setHidden(!visible);
        }
    }
}

void MainWindow::onSettingsClicked()
{
    // In a real app, this would open a settings dialog
    QMessageBox::information(this, "Settings",
                             "Settings dialog would open here.\n\n"
                             "Features would include:\n"
                             "- Profile settings\n"
                             "- Notification settings\n"
                             "- Theme customization\n"
                             "- Privacy settings");
}

void MainWindow::onNetworkResponse(const QJsonObject &response)
{
    QString action = response["action"].toString();
    QString status = response["status"].toString();

    if (action == "getContacts" && status == "success") {
        // Clear existing contacts
        contactsList->clear();

        // Process contacts
        QJsonArray contacts = response["contacts"].toArray();

        if (contacts.isEmpty()) {
            statusLabel->setText("No contacts found. Add some contacts to start chatting!");
        } else {
            for (const QJsonValue &contactValue : contacts) {
                QJsonObject contact = contactValue.toObject();

                int contactId = contact["id"].toInt();
                QString contactName = contact["username"].toString();
                QString lastMessage = contact["lastMessage"].toString();
                QString lastMessageTime = contact["lastMessageTime"].toString();
                int unreadCount = contact["unreadCount"].toInt();

                QListWidgetItem *item = new QListWidgetItem(contactsList);
                item->setSizeHint(QSize(contactsList->width(), 60));

                ContactListItem *contactWidget = new ContactListItem(
                    contactId, contactName, lastMessage, lastMessageTime, unreadCount);

                contactsList->setItemWidget(item, contactWidget);
            }
            statusLabel->setText("Contacts loaded successfully");
        }
    }
    else if (action == "getChatHistory") {
        // Clear current chat (remove loading indicator)
        while (QLayoutItem *item = chatLayout->takeAt(0)) {
            delete item->widget();
            delete item;
        }

        if (status == "success") {
            // Process chat history
            QJsonArray messages = response["messages"].toArray();

            if (messages.isEmpty()) {
                // Show "No messages yet" indicator
                QLabel *emptyLabel = new QLabel("No messages yet. Start a conversation!");
                emptyLabel->setAlignment(Qt::AlignCenter);
                chatLayout->addWidget(emptyLabel);
            } else {
                // Add messages to chat
                for (const QJsonValue &messageValue : messages) {
                    QJsonObject message = messageValue.toObject();
                    addMessageToChat(message);
                }

                statusLabel->setText("Chat history loaded successfully");
            }
        } else {
            // Show error message in chat
            QString errorMessage = response["message"].toString();
            QLabel *errorLabel = new QLabel("Failed to load messages: " + errorMessage);
            errorLabel->setAlignment(Qt::AlignCenter);
            errorLabel->setStyleSheet("color: red;");
            chatLayout->addWidget(errorLabel);

            statusLabel->setText("Error loading chat history");
        }

        // Scroll to bottom
        QTimer::singleShot(100, [this]() {
            chatScrollArea->verticalScrollBar()->setValue(
                chatScrollArea->verticalScrollBar()->maximum());
        });
    }
    else if (action == "sendMessage" && status == "success") {
        // Message sent successfully, nothing to do
        // (we already added the message to the chat optimistically)
    }
    else if (action == "addContact") {
        if (status == "success") {
            // Reload contacts
            loadContacts();
            statusLabel->setText("Contact added successfully");
        } else {
            QString errorMessage = response["message"].toString();
            QMessageBox::warning(this, "Error", "Failed to add contact: " + errorMessage);
        }
    }
    else if (status == "error") {
        QString errorMessage = response["message"].toString();
        statusLabel->setText("Error: " + errorMessage);
    }
}

void MainWindow::onMessageReceived(const QJsonObject &message)
{
    int senderId = message["senderId"].toInt();

    // Add message to chat if from current contact
    if (senderId == selectedContactId) {
        addMessageToChat(message);

        // Scroll to bottom
        QTimer::singleShot(100, [this]() {
            chatScrollArea->verticalScrollBar()->setValue(
                chatScrollArea->verticalScrollBar()->maximum());
        });
    }

    // Update contact list item
    for (int i = 0; i < contactsList->count(); ++i) {
        QListWidgetItem *item = contactsList->item(i);
        ContactListItem *contactItem = dynamic_cast<ContactListItem*>(
            contactsList->itemWidget(item));

        if (contactItem && contactItem->contactId() == senderId) {
            // Update last message
            QString content = message["content"].toString();
            QString timestamp = message["timestamp"].toString();
            QDateTime dateTime = QDateTime::fromString(timestamp, Qt::ISODate);
            QString timeStr = Utils::formatTimestamp(dateTime);

            contactItem->updateLastMessage(content, timeStr);

            // Increment unread count if not the selected contact
            if (senderId != selectedContactId) {
                contactItem->incrementUnreadCount();
            }

            // Move contact to top of list
            QListWidgetItem *takenItem = contactsList->takeItem(i);
            contactsList->insertItem(0, takenItem);
            contactsList->setItemWidget(takenItem, contactItem);
            break;
        }
    }
}

void MainWindow::addMessageToChat(const QJsonObject &message)
{
    // Check if we have all required fields
    if (!message.contains("senderId") || !message.contains("content") || !message.contains("timestamp")) {
        qDebug() << "Invalid message format:" << message;
        return;
    }

    int senderId = message["senderId"].toInt();
    QString content = message["content"].toString();
    QString timestamp = message["timestamp"].toString();
    QString type = message.contains("type") ? message["type"].toString() : "text";

    // Convert timestamp to formatted string
    QDateTime dateTime = QDateTime::fromString(timestamp, Qt::ISODate);
    QString timeStr = Utils::formatTimestamp(dateTime);

    // Create bubble widget
    bool isFromMe = (senderId == currentUserId);
    ChatBubble *bubble = new ChatBubble(content, timeStr, isFromMe, type);

    // Add to layout
    chatLayout->addWidget(bubble);
}

void MainWindow::showWelcomeScreen()
{
    rightStack->setCurrentWidget(welcomePanel);
    selectedContactId = -1;
}

void MainWindow::loadChatHistory(int contactId)
{
    // Request chat history
    QJsonObject request;
    request["action"] = "getChatHistory";
    request["userId"] = currentUserId;
    request["contactId"] = contactId;

    // Show loading status
    statusLabel->setText("Loading chat history...");

    // Send request to server
    networkClient->sendRequest(request);

    // Debug output
    qDebug() << "Requesting chat history for contact ID:" << contactId;
}

void MainWindow::onToggleTheme()
{
    isDarkTheme = !isDarkTheme;

    if (isDarkTheme) {
        loadStyleSheet(":/resources/dark.qss");
    } else {
        loadStyleSheet(":/resources/light.qss");
    }
}

void MainWindow::loadStyleSheet(const QString &path)
{
    QFile file(path);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QString styleSheet = file.readAll();
        qApp->setStyleSheet(styleSheet);
        file.close();
    }
}

void MainWindow::onLogoutClicked()
{
    // Confirm logout
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Logout", "Are you sure you want to log out?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Disconnect from server
        networkClient->disconnect();

        // Show login window
        LoginWindow *loginWindow = new LoginWindow();
        loginWindow->setAttribute(Qt::WA_DeleteOnClose);
        loginWindow->show();

        // Close this window
        close();
    }
}
