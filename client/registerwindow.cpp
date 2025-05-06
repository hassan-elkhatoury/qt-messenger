#include "registerwindow.h"
#include "loginwindow.h"
#include "utils.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QJsonObject>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QIcon>

RegisterWindow::RegisterWindow(QWidget *parent)
    : QMainWindow(parent)
    , networkClient(new NetworkClient(this))
{
    setupUI();
    
    // Connect network signals
    connect(networkClient, &NetworkClient::responseReceived, 
            this, &RegisterWindow::onNetworkResponse);
}

RegisterWindow::~RegisterWindow()
{
}

void RegisterWindow::setupUI()
{
    // Configure window
    setWindowTitle("Qt Messenger - Register");
    setMinimumSize(450, 550);
    setWindowIcon(QIcon(":/resources/icons/app_logo.png"));
    
    // Create central widget and layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // Title with back button
    QHBoxLayout *titleLayout = new QHBoxLayout();
    
    backButton = new QPushButton();
    backButton->setIcon(QIcon(":/resources/icons/back.png"));
    backButton->setFlat(true);
    backButton->setCursor(Qt::PointingHandCursor);
    backButton->setToolTip("Back to Login");
    
    QLabel *titleLabel = new QLabel("Create Account");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    
    titleLayout->addWidget(backButton);
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    
    // Registration form
    QGroupBox *registerGroup = new QGroupBox("Account Information");
    QFormLayout *formLayout = new QFormLayout(registerGroup);
    formLayout->setSpacing(15);
    
    usernameEdit = new QLineEdit();
    usernameEdit->setPlaceholderText("Choose a username");
    usernameEdit->setMinimumHeight(35);
    
    // Username validator (alphanumeric and underscore only)
    QRegularExpression usernameRegex("^[a-zA-Z0-9_]{3,20}$");
    QRegularExpressionValidator *usernameValidator = new QRegularExpressionValidator(usernameRegex, this);
    usernameEdit->setValidator(usernameValidator);
    
    emailEdit = new QLineEdit();
    emailEdit->setPlaceholderText("Your email address");
    emailEdit->setMinimumHeight(35);
    
    // Email validator
    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    QRegularExpressionValidator *emailValidator = new QRegularExpressionValidator(emailRegex, this);
    emailEdit->setValidator(emailValidator);
    
    passwordEdit = new QLineEdit();
    passwordEdit->setPlaceholderText("Create a password");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setMinimumHeight(35);
    
    confirmPasswordEdit = new QLineEdit();
    confirmPasswordEdit->setPlaceholderText("Confirm your password");
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setMinimumHeight(35);
    
    // Add fields to form
    formLayout->addRow("Username:", usernameEdit);
    formLayout->addRow("Email:", emailEdit);
    formLayout->addRow("Password:", passwordEdit);
    formLayout->addRow("Confirm Password:", confirmPasswordEdit);
    
    // Register button
    registerButton = new QPushButton("Create Account");
    registerButton->setMinimumHeight(40);
    registerButton->setCursor(Qt::PointingHandCursor);
    
    // Status label for messages
    statusLabel = new QLabel();
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setWordWrap(true);
    
    // Add widgets to main layout
    mainLayout->addLayout(titleLayout);
    mainLayout->addWidget(registerGroup);
    mainLayout->addWidget(registerButton);
    mainLayout->addWidget(statusLabel);
    mainLayout->addStretch();
    
    setCentralWidget(centralWidget);
    
    // Connect signals
    connect(registerButton, &QPushButton::clicked, this, &RegisterWindow::onRegisterClicked);
    connect(backButton, &QPushButton::clicked, this, &RegisterWindow::onBackToLoginClicked);
    connect(confirmPasswordEdit, &QLineEdit::returnPressed, this, &RegisterWindow::onRegisterClicked);
}

void RegisterWindow::onRegisterClicked()
{
    // Clear previous status
    statusLabel->clear();
    
    // Validate inputs
    QString username = usernameEdit->text().trimmed();
    QString email = emailEdit->text().trimmed();
    QString password = passwordEdit->text();
    QString confirmPassword = confirmPasswordEdit->text();
    
    // Check if all fields are filled
    if (username.isEmpty() || email.isEmpty() || password.isEmpty() || confirmPassword.isEmpty()) {
        statusLabel->setText("Please fill in all fields");
        statusLabel->setStyleSheet("color: red;");
        return;
    }
    
    // Check password match
    if (password != confirmPassword) {
        statusLabel->setText("Passwords do not match");
        statusLabel->setStyleSheet("color: red;");
        return;
    }
    
    // Check password strength (minimum 8 characters)
    if (password.length() < 8) {
        statusLabel->setText("Password must be at least 8 characters long");
        statusLabel->setStyleSheet("color: red;");
        return;
    }
    
    // Hash the password
    QString hashedPassword = Utils::hashPassword(password);
    
    // Prepare registration request
    QJsonObject request;
    request["action"] = "register";
    request["username"] = username;
    request["email"] = email;
    request["password"] = hashedPassword;
    
    // Send registration request
    registerButton->setEnabled(false);
    registerButton->setText("Creating account...");
    networkClient->sendRequest(request);
}

void RegisterWindow::onBackToLoginClicked()
{
    // Go back to login window
    QWidget *parent = parentWidget();
    if (parent) {
        parent->show();
    } else {
        LoginWindow *loginWindow = new LoginWindow();
        loginWindow->setAttribute(Qt::WA_DeleteOnClose);
        loginWindow->show();
    }
    close();
}

void RegisterWindow::onNetworkResponse(const QJsonObject &response)
{
    registerButton->setEnabled(true);
    registerButton->setText("Create Account");
    
    QString status = response["status"].toString();
    
    if (status == "success") {
        statusLabel->setText("Account created successfully! You can now login.");
        statusLabel->setStyleSheet("color: green;");
        
        // Disable inputs after successful registration
        usernameEdit->setEnabled(false);
        emailEdit->setEnabled(false);
        passwordEdit->setEnabled(false);
        confirmPasswordEdit->setEnabled(false);
        registerButton->setEnabled(false);
        
        // Automatically return to login after 2 seconds
        QTimer::singleShot(2000, this, &RegisterWindow::onBackToLoginClicked);
    } else {
        // Show error message
        QString errorMessage = response["message"].toString();
        statusLabel->setText(errorMessage);
        statusLabel->setStyleSheet("color: red;");
    }
}