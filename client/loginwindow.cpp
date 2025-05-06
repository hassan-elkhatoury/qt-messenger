#include "loginwindow.h"
#include "registerwindow.h"
#include "mainwindow.h"
#include "utils.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QPixmap>
#include <QIcon>
#include <QApplication>


LoginWindow::LoginWindow(QWidget *parent)
    : QMainWindow(parent)
    , networkClient(new NetworkClient(this)) // should match header order
    , isDarkTheme(false)
{
    setupUI();
    
    // Connect network signals
    connect(networkClient, &NetworkClient::responseReceived, 
            this, &LoginWindow::onNetworkResponse);
    
    // Load saved credentials if available
    QSettings settings;
    if (settings.contains("username") && settings.contains("rememberMe") && 
        settings.value("rememberMe").toBool()) {
        usernameEdit->setText(settings.value("username").toString());
        rememberMeCheckBox->setChecked(true);
        passwordEdit->setFocus();
    }
}

LoginWindow::~LoginWindow()
{
}

void LoginWindow::setupUI()
{
    // Configure window
    setWindowTitle("Qt Messenger - Login");
    setMinimumSize(400, 500);
    setWindowIcon(QIcon(":/resources/icons/app_logo.png"));
    
    // Create central widget and layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // App logo
    QLabel *logoLabel = new QLabel();
    QPixmap logo(":/resources/icons/app_logo.png");
    logoLabel->setPixmap(logo.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setAlignment(Qt::AlignCenter);
    
    // App title
    QLabel *titleLabel = new QLabel("Qt Messenger");
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    
    // Login form group
    QGroupBox *loginGroup = new QGroupBox("Login");
    QFormLayout *formLayout = new QFormLayout(loginGroup);
    formLayout->setSpacing(15);
    
    usernameEdit = new QLineEdit();
    usernameEdit->setPlaceholderText("Username or Email");
    usernameEdit->setMinimumHeight(35);
    
    passwordEdit = new QLineEdit();
    passwordEdit->setPlaceholderText("Password");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setMinimumHeight(35);
    
    rememberMeCheckBox = new QCheckBox("Remember Me");
    
    loginButton = new QPushButton("Login");
    loginButton->setMinimumHeight(40);
    loginButton->setCursor(Qt::PointingHandCursor);
    
    formLayout->addRow("Username:", usernameEdit);
    formLayout->addRow("Password:", passwordEdit);
    formLayout->addRow("", rememberMeCheckBox);
    formLayout->addRow(loginButton);
    
    // Registration option
    QHBoxLayout *registerLayout = new QHBoxLayout();
    QLabel *registerLabel = new QLabel("Don't have an account?");
    registerButton = new QPushButton("Register");
    registerButton->setCursor(Qt::PointingHandCursor);
    registerButton->setFlat(true);
    
    registerLayout->addWidget(registerLabel);
    registerLayout->addWidget(registerButton);
    registerLayout->setAlignment(Qt::AlignCenter);
    
    // Status label for error messages
    statusLabel = new QLabel();
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("color: red;");
    statusLabel->setWordWrap(true);
    
    // Theme toggle button
    themeButton = new QPushButton();
    themeButton->setIcon(QIcon(":/resources/icons/theme.png"));
    themeButton->setFlat(true);
    themeButton->setCursor(Qt::PointingHandCursor);
    themeButton->setToolTip("Toggle Dark/Light Theme");
    
    // Add widgets to main layout
    mainLayout->addWidget(logoLabel);
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(loginGroup);
    mainLayout->addLayout(registerLayout);
    mainLayout->addWidget(statusLabel);
    mainLayout->addStretch();
    
    // Add theme button at the bottom right
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();
    bottomLayout->addWidget(themeButton);
    mainLayout->addLayout(bottomLayout);
    
    setCentralWidget(centralWidget);
    
    // Connect signals/slots
    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(registerButton, &QPushButton::clicked, this, &LoginWindow::onRegisterClicked);
    connect(themeButton, &QPushButton::clicked, this, &LoginWindow::onToggleTheme);
    connect(passwordEdit, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);
}

void LoginWindow::onLoginClicked()
{
    // Clear previous status
    statusLabel->clear();
    
    // Validate inputs
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        statusLabel->setText("Please enter both username and password");
        return;
    }
    
    // Save credentials if remember me is checked
    QSettings settings;
    settings.setValue("username", username);
    settings.setValue("rememberMe", rememberMeCheckBox->isChecked());
    
    // Hash the password
    QString hashedPassword = Utils::hashPassword(password);
    
    // Prepare login request
    QJsonObject request;
    request["action"] = "login";
    request["username"] = username;
    request["password"] = hashedPassword;
    
    // Send login request
    loginButton->setEnabled(false);
    loginButton->setText("Logging in...");
    networkClient->sendRequest(request);
}

void LoginWindow::onRegisterClicked()
{
    RegisterWindow *registerWindow = new RegisterWindow(this);
    registerWindow->setAttribute(Qt::WA_DeleteOnClose);
    registerWindow->show();
    hide();
}

void LoginWindow::onToggleTheme()
{
    isDarkTheme = !isDarkTheme;
    
    if (isDarkTheme) {
        loadStyleSheet(":/resources/dark.qss");
    } else {
        loadStyleSheet(":/resources/light.qss");
    }
}

void LoginWindow::loadStyleSheet(const QString &path)
{
    QFile file(path);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QString styleSheet = file.readAll();
        static_cast<QApplication*>(QApplication::instance())->setStyleSheet(styleSheet);
        file.close();
    }
}



void LoginWindow::onNetworkResponse(const QJsonObject &response)
{
    loginButton->setEnabled(true);
    loginButton->setText("Login");
    
    QString status = response["status"].toString();
    
    if (status == "success") {
        // Extract user data
        QJsonObject userData = response["user"].toObject();
        int userId = userData["id"].toInt();
        QString username = userData["username"].toString();
        
        // Open main window
        MainWindow *mainWindow = new MainWindow(userId, username);
        mainWindow->setAttribute(Qt::WA_DeleteOnClose);
        mainWindow->show();
        mainWindow->loadContacts();
        hide();
    } else {
        // Show error message
        QString errorMessage = response["message"].toString();
        statusLabel->setText(errorMessage);
    }
}
