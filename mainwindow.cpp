#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QApplication>
#include <QDialog>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPair>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QScrollBar>
#include <QTextEdit>
#include <QStackedWidget>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QStringList>

#include <algorithm>
#include <memory>

#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/exception.h>
#include <vector>

namespace
{

const QList<QPair<QString, QString>> &avatarPalette()
{
    static const QList<QPair<QString, QString>> p = {
        {"#F58529", "#DD2A7B"},
        {"#515BD4", "#8134AF"},
        {"#3478D4", "#2AB7CA"},
        {"#11998E", "#38EF7D"},
        {"#FF512F", "#F09819"},
        {"#654EA3", "#EAAFC8"}
    };
    return p;
}

const std::string kDbHost = "tcp://127.0.0.1:3306";
const std::string kDbUser = "root";
const std::string kDbPass = "root";

bool contains(const std::vector<std::string> &v, const std::string &s)
{
    return std::find(v.begin(), v.end(), s) != v.end();
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      appPages(nullptr),
      dashboardPages(nullptr),
      loginPage(nullptr),
      dashboardPage(nullptr)
{
    ui->setupUi(this);

    setWindowTitle("Connectly");
    resize(1200, 800);
    setMinimumSize(950, 650);

    loginPage = ui->centralwidget;

    loginPage->setStyleSheet(R"(
        QWidget#centralwidget {
            background-color: #F2F5FA;
            font-family: "Segoe UI";
            color: #243B65;
        }
        QFrame#loginCard {
            background-color: white;
            border: 1px solid #E2E8F0;
            border-radius: 18px;
        }
        QLabel#brandTitle {
            color: #245BAA;
            font-size: 34px;
            font-weight: bold;
        }
        QLabel#brandSubtitle {
            color: #7B8798;
            font-size: 14px;
        }
        QLabel#fieldLabel {
            color: #34445E;
            font-size: 14px;
            font-weight: bold;
        }
        QLineEdit {
            background-color: #F8FAFD;
            color: #243B65;
            border: 1px solid #DCE3ED;
            border-radius: 9px;
            padding: 13px;
            font-size: 14px;
        }
        QLineEdit:focus {
            border: 2px solid #3478D4;
            background-color: white;
        }
        QPushButton {
            background-color: #3478D4;
            color: white;
            border: none;
            border-radius: 9px;
            padding: 13px;
            font-size: 15px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: #245FAF; }
        QPushButton#pushButton_2 {
            background-color: white;
            color: #3478D4;
            border: 1px solid #3478D4;
        }
        QPushButton#pushButton_2:hover { background-color: #EEF4FD; }
    )");

    QVBoxLayout *outerLayout =
        qobject_cast<QVBoxLayout *>(loginPage->layout());

    outerLayout->setContentsMargins(20, 20, 20, 20);
    outerLayout->setAlignment(Qt::AlignCenter);
    outerLayout->setSpacing(0);

    while (outerLayout->count() > 0)
    {
        QLayoutItem *item = outerLayout->takeAt(0);
        delete item;
    }

    QFrame *loginCard = new QFrame();
    loginCard->setObjectName("loginCard");
    loginCard->setFixedWidth(440);

    QVBoxLayout *cardLayout = new QVBoxLayout(loginCard);
    cardLayout->setContentsMargins(40, 40, 40, 40);
    cardLayout->setSpacing(15);

    QLabel *brandTitle = new QLabel("CONNECTLY");
    brandTitle->setObjectName("brandTitle");
    brandTitle->setAlignment(Qt::AlignCenter);

    QLabel *brandSubtitle =
        new QLabel("Connect with people. Build friendships.");
    brandSubtitle->setObjectName("brandSubtitle");
    brandSubtitle->setAlignment(Qt::AlignCenter);

    cardLayout->addWidget(brandTitle);
    cardLayout->addWidget(brandSubtitle);
    cardLayout->addSpacing(20);

    QLabel *usernameLabel = new QLabel("Username");
    usernameLabel->setObjectName("fieldLabel");

    ui->lineEdit->clear();
    ui->lineEdit->setPlaceholderText("Enter your username");
    ui->lineEdit->setMinimumHeight(48);

    cardLayout->addWidget(usernameLabel);
    cardLayout->addWidget(ui->lineEdit);

    QLabel *passwordLabel = new QLabel("Password");
    passwordLabel->setObjectName("fieldLabel");

    ui->lineEdit_2->clear();
    ui->lineEdit_2->setPlaceholderText("Enter your password");
    ui->lineEdit_2->setMinimumHeight(48);

    cardLayout->addWidget(passwordLabel);
    cardLayout->addWidget(ui->lineEdit_2);
    cardLayout->addSpacing(10);

    ui->pushButton->setText("Login");
    ui->pushButton->setMinimumHeight(48);
    cardLayout->addWidget(ui->pushButton);

    ui->pushButton_2->setText("Create New Account");
    ui->pushButton_2->setMinimumHeight(48);
    cardLayout->addWidget(ui->pushButton_2);

    QLabel *footer = new QLabel("Your space to connect and discover.");
    footer->setAlignment(Qt::AlignCenter);
    footer->setStyleSheet(
        "color: #98A3B3; font-size: 12px; margin-top: 12px;");
    cardLayout->addWidget(footer);

    outerLayout->addWidget(loginCard, 0, Qt::AlignCenter);

    appPages = new QStackedWidget(this);
    appPages->addWidget(loginPage);
    setCentralWidget(appPages);

    initializeDatabase();

    connect(ui->pushButton, &QPushButton::clicked,
            this, &MainWindow::handleLogin);
    connect(ui->pushButton_2, &QPushButton::clicked,
            this, &MainWindow::handleRegistration);
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::initializeDatabase()
{
    if (!graph.loadGraph(kDbHost, kDbUser, kDbPass))
    {
        QMessageBox::critical(this, "Database Error",
                              "Unable to load Connectly data from MySQL.");
    }
}

void MainWindow::handleLogin()
{
    QString username = ui->lineEdit->text().trimmed();
    QString password = ui->lineEdit_2->text();

    if (username.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(this, "Login",
                             "Please enter both username and password.");
        return;
    }

    loggedInUser = userSystem.loginUser(username.toStdString(),
                                        password.toStdString());

    if (loggedInUser.empty())
    {
        QMessageBox::warning(this, "Login Failed",
                             "Invalid username or password.");
        return;
    }

    showDashboard();
}
void MainWindow::handleRegistration()
{
    bool ok;

    QString username = QInputDialog::getText(
        this, "Create Account", "Enter username:",
        QLineEdit::Normal, "", &ok);

    if (!ok)
        return;

    username = username.trimmed();

    if (username.isEmpty())
    {
        QMessageBox::warning(this, "Registration",
                             "Username cannot be empty.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Create Password");
    dialog.setMinimumWidth(420);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *title = new QLabel("Create a strong password");
    title->setStyleSheet(
        "font-size: 20px; font-weight: bold; color: #243B65;");
    layout->addWidget(title);

    QLineEdit *passwordInput = new QLineEdit();
    passwordInput->setEchoMode(QLineEdit::Password);
    passwordInput->setPlaceholderText("Enter your password");
    layout->addWidget(passwordInput);

    QLabel *requirements = new QLabel();
    requirements->setText(
        "Your password must contain:\n"
        "✗ At least 8 characters\n"
        "✗ At least one uppercase letter (A-Z)\n"
        "✗ At least one lowercase letter (a-z)\n"
        "✗ At least one digit (0-9)\n"
        "✗ At least one special character (!@#$ etc.)");
    requirements->setStyleSheet("font-size: 14px; color: #C0392B;");
    layout->addWidget(requirements);

    QDialogButtonBox *buttons =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    QPushButton *createButton = buttons->button(QDialogButtonBox::Ok);
    createButton->setText("Create Account");
    createButton->setEnabled(false);
    layout->addWidget(buttons);

    connect(passwordInput, &QLineEdit::textChanged, &dialog,
            [=](const QString &password)
    {
        bool length    = password.length() >= 8;
        bool uppercase = password.contains(QRegularExpression("[A-Z]"));
        bool lowercase = password.contains(QRegularExpression("[a-z]"));
        bool digit     = password.contains(QRegularExpression("[0-9]"));
        bool special   = password.contains(QRegularExpression("[^A-Za-z0-9]"));

        QString status;
        status += length ? "✓" : "✗";
        status += " At least 8 characters\n";
        status += uppercase ? "✓" : "✗";
        status += " At least one uppercase letter (A-Z)\n";
        status += lowercase ? "✓" : "✗";
        status += " At least one lowercase letter (a-z)\n";
        status += digit ? "✓" : "✗";
        status += " At least one digit (0-9)\n";
        status += special ? "✓" : "✗";
        status += " At least one special character";

        requirements->setText(status);

        bool valid = length && uppercase && lowercase && digit && special;
        createButton->setEnabled(valid);

        requirements->setStyleSheet(
            valid ? "font-size: 14px; color: #168447;"
                  : "font-size: 14px; color: #C0392B;");
    });

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return;

    QString password = passwordInput->text();

    bool interestsOk = false;
    QString interests = QInputDialog::getText(
        this, "Your Interests",
        "Enter interests separated by commas:",
        QLineEdit::Normal, "", &interestsOk);

    if (!interestsOk)
        return;

    bool success = userSystem.registerUser(
        graph, username.toStdString(), password.toStdString(),
        interests.toStdString());

    if (success)
        QMessageBox::information(this, "Registration Successful",
                                 "Your Connectly account has been created!");
    else
        QMessageBox::warning(
            this, "Registration Failed",
            "Could not create account. The username may already exist.");
}
void MainWindow::logout()
{
    loggedInUser.clear();
    currentProfileUser.clear();

    ui->lineEdit->clear();
    ui->lineEdit_2->clear();

    appPages->setCurrentWidget(loginPage);
}


void MainWindow::showDashboard()
{
    if (dashboardPage != nullptr)
    {
        appPages->removeWidget(dashboardPage);
        delete dashboardPage;
        dashboardPage = nullptr;
        dashboardPages = nullptr;
        homePageWidget = nullptr;
        profilePageWidget = nullptr;
        homeLayout = nullptr;
        profileLayout = nullptr;
        searchBox = nullptr;
        searchPopup = nullptr;
        popupLayout = nullptr;
        bellButton = nullptr;
        chatButton = nullptr;
        incomingRequests.clear();
        outgoingRequests.clear();
    }

    dashboardPage = createDashboard();
    appPages->addWidget(dashboardPage);
    appPages->setCurrentWidget(dashboardPage);

    showHome();
}
QWidget *MainWindow::createDashboard()
{
    QWidget *dashboard = new QWidget();
    dashboard->setObjectName("dashboard");

    dashboard->setStyleSheet(R"(
        QWidget#dashboard {
            background-color: #F4F6FB;
            font-family: "Segoe UI";
            color: #24324A;
        }

        /* ---------- Top bar ---------- */
        QWidget#topbar {
            background-color: white;
            border-bottom: 1px solid #E6EBF3;
        }
        QPushButton#logoButton {
            background: transparent;
            border: none;
            color: #245BAA;
            font-size: 26px;
            font-weight: 800;
            padding: 4px 8px;
        }
        QPushButton#logoButton:hover { color: #3478D4; }

        QLineEdit#searchBox {
            background-color: #F0F3F9;
            border: 1px solid transparent;
            border-radius: 20px;
            padding: 10px 18px;
            font-size: 14px;
            color: #243B65;
        }
        QLineEdit#searchBox:focus {
            background-color: white;
            border: 1px solid #3478D4;
        }

        QPushButton#logoutButton {
            background: transparent;
            border: 1px solid #DCE3ED;
            border-radius: 18px;
            color: #5B6B84;
            min-height: 34px;
            max-height: 34px;
            padding: 0 16px;
            font-size: 13px;
            font-weight: 600;
        }
        QPushButton#logoutButton:hover {
            background-color: #FDECEC;
            border-color: #F3B8B8;
            color: #C0392B;
        }

        /* ---------- Content ---------- */
        QScrollArea { border: none; background: transparent; }
        QScrollArea > QWidget > QWidget { background: transparent; }

        QFrame#card {
            background-color: white;
            border: 1px solid #E6EBF3;
            border-radius: 18px;
        }
        QFrame#banner {
            border-radius: 20px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                        stop:0 #245BAA, stop:1 #4C8DF0);
        }
        QLabel#sectionTitle {
            font-size: 20px;
            font-weight: bold;
            color: #243B65;
        }
        QLabel#muted {
            color: #7B8798;
            font-size: 13px;
        }
        QLabel#cardName, QPushButton#nameButton {
            color: #243B65;
        }
        QPushButton#nameButton {
            background: transparent;
            border: none;
            font-size: 15px;
            font-weight: bold;
            padding: 2px;
        }
        QPushButton#nameButton:hover { color: #3478D4; }

        QPushButton#primaryBtn {
            background-color: #3478D4;
            color: white;
            border: none;
            border-radius: 10px;
            padding: 9px 18px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton#primaryBtn:hover { background-color: #245FAF; }

        QPushButton#ghostBtn {
            background-color: #EEF3FC;
            color: #2455A4;
            border: none;
            border-radius: 10px;
            padding: 9px 18px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton#ghostBtn:hover { background-color: #DDE8FA; }

        QPushButton#dangerBtn {
            background-color: transparent;
            color: #C0392B;
            border: 1px solid #F0C4C0;
            border-radius: 10px;
            padding: 7px 14px;
            font-size: 12px;
            font-weight: bold;
        }
        QPushButton#dangerBtn:hover { background-color: #FDECEC; }

        /* ---------- Notification bell ---------- */
        QPushButton#bellButton {
            background-color: #F0F3F9;
            border: none;
            border-radius: 18px;
            min-height: 36px;
            max-height: 36px;
            padding: 0 14px;
            font-size: 14px;
            font-weight: 600;
            color: #5B6B84;
        }
        QPushButton#bellButton:hover { background-color: #E1E9F7; }
        QPushButton#chatButton {
            background-color: #F0F3F9;
            border: none;
            border-radius: 18px;
            min-height: 36px;
            max-height: 36px;
            min-width: 42px;
            padding: 0 12px;
            font-size: 17px;
            color: #245BAA;
        }
        QPushButton#chatButton:hover { background-color: #E1E9F7; }
        QPushButton#bellButton[alert="true"] {
            background-color: #FDECEC;
            color: #C0392B;
        }
        QPushButton#bellButton[alert="true"]:hover { background-color: #FAD9D9; }

        /* ---------- Search popup ---------- */
        QFrame#popup {
            background-color: white;
            border: 1px solid #E1E7F0;
            border-radius: 16px;
        }
        QPushButton#resultName {
            background: transparent;
            border: none;
            text-align: left;
            color: #243B65;
            font-size: 14px;
            font-weight: bold;
            padding: 6px;
        }
        QPushButton#resultName:hover { color: #3478D4; }

        QScrollBar:vertical { width: 10px; background: transparent; }
        QScrollBar::handle:vertical {
            background: #CBD5E4; border-radius: 5px; min-height: 30px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QScrollBar:horizontal { height: 10px; background: transparent; }
        QScrollBar::handle:horizontal {
            background: #CBD5E4; border-radius: 5px; min-width: 30px;
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(dashboard);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(createTopBar(dashboard));

    dashboardPages = new QStackedWidget();
    homePageWidget = createScrollPage(homeLayout);
    profilePageWidget = createScrollPage(profileLayout);
    dashboardPages->addWidget(homePageWidget);
    dashboardPages->addWidget(profilePageWidget);

    mainLayout->addWidget(dashboardPages, 1);

  
    searchPopup = new QFrame(dashboard);
    searchPopup->setObjectName("popup");
    searchPopup->hide();

    auto *shadow = new QGraphicsDropShadowEffect(searchPopup);
    shadow->setBlurRadius(30);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(30, 50, 90, 55));
    searchPopup->setGraphicsEffect(shadow);

    popupLayout = new QVBoxLayout(searchPopup);
    popupLayout->setContentsMargins(10, 10, 10, 10);
    popupLayout->setSpacing(4);


    QTimer *poll = new QTimer(dashboard);
    connect(poll, &QTimer::timeout, this, &MainWindow::pollServer);
    poll->start(5000);

    return dashboard;
}
QWidget *MainWindow::createTopBar(QWidget *dashboard)
{
    Q_UNUSED(dashboard);

    QWidget *topbar = new QWidget();
    topbar->setObjectName("topbar");
    topbar->setFixedHeight(72);

    QHBoxLayout *layout = new QHBoxLayout(topbar);
    layout->setContentsMargins(24, 10, 24, 10);
    layout->setSpacing(14);

    QPushButton *logo = new QPushButton("Connectly");
    logo->setObjectName("logoButton");
    logo->setCursor(Qt::PointingHandCursor);
    logo->setToolTip("Go to Home");
    connect(logo, &QPushButton::clicked, this, &MainWindow::showHome);

    searchBox = new QLineEdit();
    searchBox->setObjectName("searchBox");
    searchBox->setPlaceholderText("🔍  Search people on Connectly");
    searchBox->setFixedWidth(440);
    searchBox->setClearButtonEnabled(true);
    searchBox->installEventFilter(this);

    connect(searchBox, &QLineEdit::textChanged, this, &MainWindow::runSearch);
    connect(searchBox, &QLineEdit::returnPressed, this, &MainWindow::runSearch);

    
    chatButton = new QPushButton("💬");
    chatButton->setObjectName("chatButton");
    chatButton->setCursor(Qt::PointingHandCursor);
    chatButton->setToolTip("Open messages");
    connect(chatButton, &QPushButton::clicked,
            this, &MainWindow::showChatDialog);

    
    bellButton = new QPushButton("🔔");
    bellButton->setObjectName("bellButton");
    bellButton->setCursor(Qt::PointingHandCursor);
    bellButton->setToolTip("Friend requests");
    connect(bellButton, &QPushButton::clicked, this, &MainWindow::showHome);

   
    QPushButton *me = createAvatar(QString::fromStdString(loggedInUser), 44);
    me->setToolTip("My profile");
    connect(me, &QPushButton::clicked, this, [this]()
    {
        showProfile(loggedInUser);
    });

    QPushButton *logoutBtn = new QPushButton("Logout");
    logoutBtn->setObjectName("logoutButton");
    logoutBtn->setCursor(Qt::PointingHandCursor);
    connect(logoutBtn, &QPushButton::clicked, this, &MainWindow::logout);

    layout->addWidget(logo);
    layout->addStretch(1);
    layout->addWidget(searchBox);
    layout->addStretch(1);
    layout->addWidget(chatButton);
    layout->addWidget(bellButton);
    layout->addWidget(me);
    layout->addWidget(logoutBtn);

    return topbar;
}
QWidget *MainWindow::createScrollPage(QVBoxLayout *&contentLayout)
{
    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    QWidget *wrapper = new QWidget();
    QHBoxLayout *wrapLayout = new QHBoxLayout(wrapper);
    wrapLayout->setContentsMargins(20, 24, 20, 24);

    QWidget *content = new QWidget();
    content->setMaximumWidth(1000);
    content->setMinimumWidth(600);

    contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(22);

    wrapLayout->addStretch(1);
    wrapLayout->addWidget(content, 100);
    wrapLayout->addStretch(1);

    scroll->setWidget(wrapper);
    return scroll;
}

void MainWindow::showChatDialog()
{
    if (loggedInUser.empty())
        return;

    QDialog *dialog = new QDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle("Connectly Messages");
    dialog->resize(900, 620);
    dialog->setMinimumSize(720, 480);

    dialog->setStyleSheet(R"(
        QDialog {
            background: #F4F6FB;
            font-family: "Segoe UI";
            color: #24324A;
        }
        QFrame#chatPanel {
            background: white;
            border: 1px solid #E6EBF3;
            border-radius: 14px;
        }
        QLabel#chatHeading {
            color: #243B65;
            font-size: 19px;
            font-weight: bold;
        }
        QLabel#chatSubheading {
            color: #7B8798;
            font-size: 12px;
        }
        QListWidget {
            background: white;
            border: none;
            outline: none;
            padding: 6px;
        }
        QListWidget::item {
            padding: 13px 10px;
            border-radius: 9px;
            color: #243B65;
        }
        QListWidget::item:selected {
            background: #EAF1FD;
            color: #245BAA;
        }
        QLineEdit#messageInput {
            background: #F7F9FC;
            border: 1px solid #DCE3ED;
            border-radius: 18px;
            padding: 11px 15px;
            color: #243B65;
            font-size: 13px;
        }
        QPushButton#sendMessageButton {
            background: #3478D4;
            color: white;
            border: none;
            border-radius: 18px;
            padding: 10px 22px;
            font-weight: bold;
        }
        QPushButton#sendMessageButton:hover { background: #245FAF; }
        QScrollArea { border: none; background: transparent; }
    )");

    QHBoxLayout *root = new QHBoxLayout(dialog);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(14);


    QFrame *leftPanel = new QFrame();
    leftPanel->setObjectName("chatPanel");
    leftPanel->setFixedWidth(255);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(14, 18, 14, 14);
    leftLayout->setSpacing(10);

    QLabel *messagesTitle = new QLabel("Messages");
    messagesTitle->setObjectName("chatHeading");
    QLabel *messagesSubtitle = new QLabel("Your friends on Connectly");
    messagesSubtitle->setObjectName("chatSubheading");

    QListWidget *friendList = new QListWidget();
    friendList->setSpacing(3);

    leftLayout->addWidget(messagesTitle);
    leftLayout->addWidget(messagesSubtitle);
    leftLayout->addSpacing(6);
    leftLayout->addWidget(friendList, 1);

  
    QFrame *rightPanel = new QFrame();
    rightPanel->setObjectName("chatPanel");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(18, 16, 18, 16);
    rightLayout->setSpacing(12);

    QHBoxLayout *conversationHeader = new QHBoxLayout();
    QLabel *conversationAvatar = new QLabel("?");
    conversationAvatar->setFixedSize(42, 42);
    conversationAvatar->setAlignment(Qt::AlignCenter);
    conversationAvatar->setStyleSheet(
        "background:#3478D4; color:white; border-radius:21px;"
        "font-size:18px; font-weight:bold;");

    QVBoxLayout *conversationTitleLayout = new QVBoxLayout();
    conversationTitleLayout->setSpacing(2);
    QLabel *conversationName = new QLabel("Select a friend");
    conversationName->setObjectName("chatHeading");
    QLabel *conversationStatus = new QLabel("Choose someone to start chatting");
    conversationStatus->setObjectName("chatSubheading");
    conversationTitleLayout->addWidget(conversationName);
    conversationTitleLayout->addWidget(conversationStatus);

    conversationHeader->addWidget(conversationAvatar);
    conversationHeader->addLayout(conversationTitleLayout, 1);
    rightLayout->addLayout(conversationHeader);

    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("color:#E6EBF3;");
    rightLayout->addWidget(separator);

    QScrollArea *messageScroll = new QScrollArea();
    messageScroll->setWidgetResizable(true);
    messageScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *messageHost = new QWidget();
    QVBoxLayout *messageLayout = new QVBoxLayout(messageHost);
    messageLayout->setContentsMargins(8, 8, 8, 8);
    messageLayout->setSpacing(10);
    messageLayout->addStretch(1);
    messageScroll->setWidget(messageHost);
    rightLayout->addWidget(messageScroll, 1);

    QHBoxLayout *composer = new QHBoxLayout();
    QLineEdit *messageInput = new QLineEdit();
    messageInput->setObjectName("messageInput");
    messageInput->setPlaceholderText("Write a message...");
    QPushButton *sendButton = new QPushButton("Send");
    sendButton->setObjectName("sendMessageButton");
    composer->addWidget(messageInput, 1);
    composer->addWidget(sendButton);
    rightLayout->addLayout(composer);

    root->addWidget(leftPanel);
    root->addWidget(rightPanel, 1);

  
    const std::vector<std::string> friends = graph.getFriends(loggedInUser);
    for (const std::string &friendName : friends)
        friendList->addItem(QString::fromStdString(friendName));

    std::string selectedFriend;

   
    auto loadConversation = [this, dialog, messageLayout, messageHost,
                             messageScroll, conversationName,
                             conversationStatus, conversationAvatar,
                             &selectedFriend]()
    {
        if (selectedFriend.empty())
            return;

        try
        {
            sql::mysql::MySQL_Driver *driver =
                sql::mysql::get_mysql_driver_instance();

            std::unique_ptr<sql::Connection> con(
                driver->connect(kDbHost, kDbUser, kDbPass));
            con->setSchema("connectly");

            std::unique_ptr<sql::PreparedStatement> stmt(
                con->prepareStatement(
                    "SELECT sender, message_text, sent_at "
                    "FROM messages "
                    "WHERE (sender=? AND receiver=?) "
                    "OR (sender=? AND receiver=?) "
                    "ORDER BY sent_at ASC, message_id ASC"));

            stmt->setString(1, loggedInUser);
            stmt->setString(2, selectedFriend);
            stmt->setString(3, selectedFriend);
            stmt->setString(4, loggedInUser);

            std::unique_ptr<sql::ResultSet> result(stmt->executeQuery());

            // Remove old bubbles but keep the stretch at the bottom.
            while (QLayoutItem *item = messageLayout->takeAt(0))
            {
                if (QWidget *widget = item->widget())
                    widget->deleteLater();
                delete item;
            }

            bool hasMessages = false;
            while (result->next())
            {
                hasMessages = true;
                const QString sender =
                    QString::fromStdString(result->getString("sender"));
                const QString body =
                    QString::fromStdString(result->getString("message_text"));
                const QString time =
                    QString::fromStdString(result->getString("sent_at"));

                QWidget *row = new QWidget(messageHost);
                QHBoxLayout *rowLayout = new QHBoxLayout(row);
                rowLayout->setContentsMargins(0, 0, 0, 0);

                QLabel *bubble = new QLabel(body);
                bubble->setWordWrap(true);
                bubble->setTextInteractionFlags(Qt::TextSelectableByMouse);
                bubble->setMaximumWidth(430);
                bubble->setContentsMargins(12, 9, 12, 5);

                QLabel *timeLabel = new QLabel(time);
                timeLabel->setStyleSheet("color:#7B8798; font-size:10px;");

                QVBoxLayout *bubbleLayout = new QVBoxLayout();
                bubbleLayout->setContentsMargins(0, 0, 0, 0);
                bubbleLayout->setSpacing(3);
                bubbleLayout->addWidget(bubble);
                bubbleLayout->addWidget(timeLabel,
                    sender == QString::fromStdString(loggedInUser)
                        ? Qt::AlignRight : Qt::AlignLeft);

                if (sender == QString::fromStdString(loggedInUser))
                {
                    bubble->setStyleSheet(
                        "background:#3478D4; color:white;"
                        "border-radius:13px;");
                    rowLayout->addStretch();
                    rowLayout->addLayout(bubbleLayout);
                }
                else
                {
                    bubble->setStyleSheet(
                        "background:#EEF2F8; color:#243B65;"
                        "border-radius:13px;");
                    rowLayout->addLayout(bubbleLayout);
                    rowLayout->addStretch();
                }

                messageLayout->addWidget(row);
            }

            if (!hasMessages)
            {
                QLabel *empty = new QLabel(
                    "No messages yet. Say hello to start the conversation!");
                empty->setStyleSheet("color:#7B8798; padding:20px;");
                empty->setAlignment(Qt::AlignCenter);
                messageLayout->addWidget(empty);
            }

            messageLayout->addStretch(1);
            conversationName->setText(
                QString::fromStdString(selectedFriend));
            conversationStatus->setText("Connectly friend");
            conversationAvatar->setText(
                QString::fromStdString(selectedFriend).left(1).toUpper());

          
            std::unique_ptr<sql::PreparedStatement> readStmt(
                con->prepareStatement(
                    "UPDATE messages SET is_read=TRUE "
                    "WHERE sender=? AND receiver=? AND is_read=FALSE"));
            readStmt->setString(1, selectedFriend);
            readStmt->setString(2, loggedInUser);
            readStmt->executeUpdate();

            QTimer::singleShot(0, messageScroll, [messageScroll]()
            {
                messageScroll->verticalScrollBar()->setValue(
                    messageScroll->verticalScrollBar()->maximum());
            });
        }
        catch (sql::SQLException &e)
        {
            QMessageBox::warning(dialog, "Chat Database Error",
                QString("Could not load messages:\n%1")
                    .arg(QString::fromStdString(e.what())));
        }
    };

    connect(friendList, &QListWidget::currentTextChanged, dialog,
            [&, loadConversation](const QString &name)
    {
        selectedFriend = name.toStdString();
        loadConversation();
    });

    auto sendCurrentMessage = [this, dialog, messageInput,
                               &selectedFriend, loadConversation]()
    {
        if (selectedFriend.empty())
        {
            QMessageBox::information(dialog, "Messages",
                                     "Select a friend first.");
            return;
        }

        const QString message = messageInput->text().trimmed();
        if (message.isEmpty())
            return;

        try
        {
            sql::mysql::MySQL_Driver *driver =
                sql::mysql::get_mysql_driver_instance();

            std::unique_ptr<sql::Connection> con(
                driver->connect(kDbHost, kDbUser, kDbPass));
            con->setSchema("connectly");

            std::unique_ptr<sql::PreparedStatement> stmt(
                con->prepareStatement(
                    "INSERT INTO messages "
                    "(sender, receiver, message_text) VALUES (?, ?, ?)"));

            stmt->setString(1, loggedInUser);
            stmt->setString(2, selectedFriend);
            stmt->setString(3, message.toStdString());
            stmt->executeUpdate();

            messageInput->clear();
            loadConversation();
        }
        catch (sql::SQLException &e)
        {
            QMessageBox::warning(dialog, "Send Failed",
                QString("Could not send message:\n%1")
                    .arg(QString::fromStdString(e.what())));
        }
    };

    connect(sendButton, &QPushButton::clicked,
            dialog, sendCurrentMessage);
    connect(messageInput, &QLineEdit::returnPressed,
            dialog, sendCurrentMessage);

    // Refresh the open conversation so replies appear without reopening it.
    QTimer *refreshTimer = new QTimer(dialog);
    connect(refreshTimer, &QTimer::timeout, dialog, loadConversation);
    refreshTimer->start(2000);

    if (friendList->count() > 0)
        friendList->setCurrentRow(0);
    else
    {
        conversationName->setText("No friends yet");
        conversationStatus->setText(
            "You can message people after they become your accepted friends.");
        QLabel *empty = new QLabel("Your accepted friends will appear here.");
        empty->setAlignment(Qt::AlignCenter);
        empty->setStyleSheet("color:#7B8798; padding:24px;");
        messageLayout->insertWidget(0, empty);
    }

    dialog->exec();
}

void MainWindow::showHome()
{
    if (searchPopup) searchPopup->hide();

    if (searchBox)
    {
        searchBox->blockSignals(true);
        searchBox->clear();
        searchBox->blockSignals(false);
    }

    loadRequests();
    populateHome();
    dashboardPages->setCurrentWidget(homePageWidget);
}

void MainWindow::showProfile(const std::string &user)
{
    currentProfileUser = user;

    if (searchPopup) searchPopup->hide();

    if (searchBox)
    {
        searchBox->blockSignals(true);
        searchBox->clear();
        searchBox->blockSignals(false);
    }

    loadRequests();
    populateProfile();
    dashboardPages->setCurrentWidget(profilePageWidget);
}
void MainWindow::populateHome()
{
    clearLayout(homeLayout);

    const QString me = QString::fromStdString(loggedInUser);

    // ----- Welcome banner -----
    QFrame *banner = new QFrame();
    banner->setObjectName("banner");
    QVBoxLayout *bannerLayout = new QVBoxLayout(banner);
    bannerLayout->setContentsMargins(28, 24, 28, 24);
    bannerLayout->setSpacing(4);

    QLabel *hello = new QLabel("Welcome back, " + me + " 👋");
    hello->setStyleSheet(
        "background: transparent; color: white;"
        "font-size: 26px; font-weight: bold;");

    QLabel *sub = new QLabel(
        "See who's around, catch up with friends and grow your circle.");
    sub->setStyleSheet(
        "background: transparent; color: #DCE8FF; font-size: 14px;");

    bannerLayout->addWidget(hello);
    bannerLayout->addWidget(sub);
    homeLayout->addWidget(banner);

   
    if (!incomingRequests.empty())
    {
        QLabel *reqTitle = new QLabel(
            QString("Friend requests (%1)").arg(incomingRequests.size()));
        reqTitle->setObjectName("sectionTitle");
        homeLayout->addWidget(reqTitle);

        for (const std::string &r : incomingRequests)
        {
            QString rname = QString::fromStdString(r);

            QFrame *row = new QFrame();
            row->setObjectName("card");

            QHBoxLayout *rl = new QHBoxLayout(row);
            rl->setContentsMargins(18, 14, 18, 14);
            rl->setSpacing(14);

            QPushButton *av = createAvatar(rname, 52);

            QVBoxLayout *text = new QVBoxLayout();
            text->setSpacing(0);

            QPushButton *nameBtn = new QPushButton(rname);
            nameBtn->setObjectName("nameButton");
            nameBtn->setCursor(Qt::PointingHandCursor);

            QLabel *sub = new QLabel(
                QString::number(mutualCount(r)) +
                " mutual friend(s)  ·  wants to connect with you");
            sub->setObjectName("muted");

            text->addWidget(nameBtn, 0, Qt::AlignLeft);
            text->addWidget(sub);

            rl->addWidget(av);
            rl->addLayout(text, 1);
            rl->addWidget(createRelationWidget(r, ""));   // Accept / Reject

            connect(av, &QPushButton::clicked, this, [this, r]()
            {
                showProfile(r);
            });
            connect(nameBtn, &QPushButton::clicked, this, [this, r]()
            {
                showProfile(r);
            });

            homeLayout->addWidget(row);
        }
    }

  
    if (!outgoingRequests.empty())
    {
        QLabel *sentTitle = new QLabel("Sent requests");
        sentTitle->setObjectName("sectionTitle");
        homeLayout->addWidget(sentTitle);

        QFrame *box = new QFrame();
        box->setObjectName("card");
        QVBoxLayout *bl = new QVBoxLayout(box);
        bl->setContentsMargins(18, 12, 18, 12);
        bl->setSpacing(8);

        for (const std::string &r : outgoingRequests)
        {
            QString rname = QString::fromStdString(r);

            QHBoxLayout *rl = new QHBoxLayout();
            rl->setSpacing(12);

            QPushButton *av = createAvatar(rname, 40);
            QPushButton *nameBtn = new QPushButton(rname);
            nameBtn->setObjectName("resultName");
            nameBtn->setCursor(Qt::PointingHandCursor);

            QLabel *pending = new QLabel("Waiting for response");
            pending->setObjectName("muted");

            QPushButton *cancel = new QPushButton("Cancel request");
            cancel->setObjectName("dangerBtn");
            cancel->setCursor(Qt::PointingHandCursor);
            cancel->setFocusPolicy(Qt::NoFocus);

            rl->addWidget(av);
            rl->addWidget(nameBtn);
            rl->addWidget(pending);
            rl->addStretch(1);
            rl->addWidget(cancel);

            connect(av, &QPushButton::clicked, this, [this, r]()
            {
                showProfile(r);
            });
            connect(nameBtn, &QPushButton::clicked, this, [this, r]()
            {
                showProfile(r);
            });
            connect(cancel, &QPushButton::clicked, this, [this, r]()
            {
                cancelRequest(r);
            });

            bl->addLayout(rl);
        }

        homeLayout->addWidget(box);
    }

  
    std::vector<std::string> friends = graph.getFriends(loggedInUser);

    QLabel *friendsTitle = new QLabel("Your friends");
    friendsTitle->setObjectName("sectionTitle");
    homeLayout->addWidget(friendsTitle);

    if (friends.empty())
    {
        QLabel *none = new QLabel(
            "You don't have any friends yet — add someone from the "
            "suggestions below or use the search bar.");
        none->setObjectName("muted");
        none->setWordWrap(true);
        homeLayout->addWidget(none);
    }
    else
    {
        QScrollArea *row = new QScrollArea();
        row->setFrameShape(QFrame::NoFrame);
        row->setWidgetResizable(true);
        row->setFixedHeight(128);
        row->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        QWidget *rowContent = new QWidget();
        QHBoxLayout *rowLayout = new QHBoxLayout(rowContent);
        rowLayout->setContentsMargins(2, 4, 2, 4);
        rowLayout->setSpacing(22);

        for (const std::string &f : friends)
        {
            QWidget *item = new QWidget();
            QVBoxLayout *col = new QVBoxLayout(item);
            col->setContentsMargins(0, 0, 0, 0);
            col->setSpacing(6);

            QString name = QString::fromStdString(f);

            QPushButton *avatar = createAvatar(name, 74);
            QLabel *label = new QLabel(name);
            label->setAlignment(Qt::AlignCenter);
            label->setStyleSheet(
                "color: #34445E; font-size: 13px; font-weight: 600;");

            col->addWidget(avatar, 0, Qt::AlignHCenter);
            col->addWidget(label);

            connect(avatar, &QPushButton::clicked, this, [this, f]()
            {
                showProfile(f);
            });

            rowLayout->addWidget(item);
        }
        rowLayout->addStretch();

        row->setWidget(rowContent);
        homeLayout->addWidget(row);
    }

  
    QLabel *sugTitle = new QLabel("Suggested for you");
    sugTitle->setObjectName("sectionTitle");
    homeLayout->addWidget(sugTitle);

    auto recommendations = suggestion.suggestTopK(graph, loggedInUser);

    if (recommendations.empty())
    {
        QLabel *none = new QLabel("No new friend suggestions right now.");
        none->setObjectName("muted");
        homeLayout->addWidget(none);
    }
    else
    {
        QWidget *gridHost = new QWidget();
        QGridLayout *grid = new QGridLayout(gridHost);
        grid->setContentsMargins(0, 0, 0, 0);
        grid->setSpacing(16);

        int idx = 0;
        for (const auto &person : recommendations)
        {
            std::string pname = person.username;
            QString display = QString::fromStdString(pname);

            QFrame *card = new QFrame();
            card->setObjectName("card");

            QVBoxLayout *cl = new QVBoxLayout(card);
            cl->setContentsMargins(18, 20, 18, 18);
            cl->setSpacing(6);

            QPushButton *avatar = createAvatar(display, 80);
            QPushButton *nameBtn = new QPushButton(display);
            nameBtn->setObjectName("nameButton");
            nameBtn->setCursor(Qt::PointingHandCursor);

            QLabel *mutual = new QLabel(
                QString::number(person.mutualCount) + " mutual friend(s)");
            mutual->setObjectName("muted");
            mutual->setAlignment(Qt::AlignCenter);

            QString sharedText;
            if (person.sharedInterests.empty())
            {
                sharedText = "Shared interests: None";
            }
            else
            {
                QStringList sharedNames;
                for (const std::string &interest : person.sharedInterests)
                    sharedNames << QString::fromStdString(interest);

                sharedText = "Shared interests: " + sharedNames.join(", ");
            }

            QLabel *interestsLabel = new QLabel(sharedText);
            interestsLabel->setObjectName("muted");
            interestsLabel->setWordWrap(true);
            interestsLabel->setAlignment(Qt::AlignCenter);

            QLabel *score = new QLabel(
                "Suggestion score: " +
                QString::number(person.mutualCount + person.interestCount));
            score->setObjectName("muted");
            score->setAlignment(Qt::AlignCenter);

            cl->addWidget(avatar, 0, Qt::AlignHCenter);
            cl->addWidget(nameBtn);
            cl->addWidget(mutual);
            cl->addWidget(interestsLabel);
            cl->addWidget(score);
            cl->addSpacing(8);
            cl->addWidget(createRelationWidget(pname, "Add Friend"));

            connect(avatar, &QPushButton::clicked, this, [this, pname]()
            {
                showProfile(pname);
            });
            connect(nameBtn, &QPushButton::clicked, this, [this, pname]()
            {
                showProfile(pname);
            });

            grid->addWidget(card, idx / 3, idx % 3);
            ++idx;
        }
        for (int c = 0; c < 3; ++c)
            grid->setColumnStretch(c, 1);

        homeLayout->addWidget(gridHost);
    }

    homeLayout->addStretch();
}

void MainWindow::populateProfile()
{
    clearLayout(profileLayout);

    const std::string user = currentProfileUser;
    const QString name = QString::fromStdString(user);
    const bool own = (user == loggedInUser);

    std::vector<std::string> friends = graph.getFriends(user);

  
    QFrame *header = new QFrame();
    header->setObjectName("card");

    QHBoxLayout *hl = new QHBoxLayout(header);
    hl->setContentsMargins(36, 30, 36, 30);
    hl->setSpacing(40);

    QPushButton *avatar = createAvatar(name, 128);
    avatar->setCursor(Qt::ArrowCursor);
    hl->addWidget(avatar, 0, Qt::AlignTop);

    QVBoxLayout *info = new QVBoxLayout();
    info->setSpacing(10);

    QHBoxLayout *nameRow = new QHBoxLayout();
    nameRow->setSpacing(14);

    QLabel *nameLabel = new QLabel(name);
    nameLabel->setStyleSheet(
        "font-size: 28px; font-weight: bold; color: #243B65;");
    nameRow->addWidget(nameLabel);

    if (own)
    {
        QLabel *you = new QLabel("You");
        you->setStyleSheet(
            "background: #E8F0FE; color: #2455A4; border-radius: 10px;"
            "padding: 3px 12px; font-size: 12px; font-weight: bold;");
        nameRow->addWidget(you);
    }
    else
    {
        nameRow->addWidget(createRelationWidget(user, "+ Add Friend"));
    }
    nameRow->addStretch();
    info->addLayout(nameRow);


    QHBoxLayout *stats = new QHBoxLayout();
    stats->setSpacing(32);

    auto makeStat = [](const QString &num, const QString &label)
    {
        QLabel *l = new QLabel(
            "<span style='font-size:20px; font-weight:bold; color:#243B65;'>" +
            num + "</span>&nbsp;<span style='font-size:14px; color:#7B8798;'>" +
            label + "</span>");
        return l;
    };

    stats->addWidget(makeStat(QString::number(friends.size()), "friends"));
    if (!own)
        stats->addWidget(makeStat(QString::number(mutualCount(user)),
                                  "mutual"));
    stats->addStretch();
    info->addLayout(stats);

    QLabel *bio = new QLabel(own ? "Your Connectly profile."
                                 : "Connectly member");
    bio->setObjectName("muted");
    info->addWidget(bio);
    info->addStretch();

    hl->addLayout(info, 1);
    profileLayout->addWidget(header);

    QLabel *title = new QLabel(own ? "My friends" : name + "'s friends");
    title->setObjectName("sectionTitle");
    profileLayout->addWidget(title);

    if (friends.empty())
    {
        QLabel *none = new QLabel(own ? "You don't have any friends yet."
                                      : "No friends to show.");
        none->setObjectName("muted");
        profileLayout->addWidget(none);
    }
    else
    {
        QWidget *gridHost = new QWidget();
        QGridLayout *grid = new QGridLayout(gridHost);
        grid->setContentsMargins(0, 0, 0, 0);
        grid->setSpacing(16);

        int idx = 0;
        for (const std::string &f : friends)
        {
            QString fname = QString::fromStdString(f);

            QFrame *card = new QFrame();
            card->setObjectName("card");

            QVBoxLayout *cl = new QVBoxLayout(card);
            cl->setContentsMargins(14, 18, 14, 14);
            cl->setSpacing(6);

            QPushButton *fav = createAvatar(fname, 64);
            QPushButton *nameBtn = new QPushButton(fname);
            nameBtn->setObjectName("nameButton");
            nameBtn->setCursor(Qt::PointingHandCursor);

            cl->addWidget(fav, 0, Qt::AlignHCenter);
            cl->addWidget(nameBtn);

            connect(fav, &QPushButton::clicked, this, [this, f]()
            {
                showProfile(f);
            });
            connect(nameBtn, &QPushButton::clicked, this, [this, f]()
            {
                showProfile(f);
            });

            if (own)
            {
                QPushButton *rm = new QPushButton("Remove");
                rm->setObjectName("dangerBtn");
                rm->setCursor(Qt::PointingHandCursor);
                connect(rm, &QPushButton::clicked, this, [this, f]()
                {
                    removeFriend(f);
                });
                cl->addWidget(rm);
            }

            grid->addWidget(card, idx / 4, idx % 4);
            ++idx;
        }
        for (int c = 0; c < 4; ++c)
            grid->setColumnStretch(c, 1);

        profileLayout->addWidget(gridHost);
    }

    profileLayout->addStretch();
}

void MainWindow::runSearch()
{
    if (!searchBox || !searchPopup || !popupLayout)
        return;

    clearLayout(popupLayout);

    QString text = searchBox->text().trimmed();
    if (text.isEmpty())
    {
        searchPopup->hide();
        return;
    }

    std::string uname = text.toStdString();
    bool found = graph.searchPerson(uname);

    if (!found)
    {
        QLabel *none = new QLabel("No user found for “" + text + "”");
        none->setObjectName("muted");
        none->setContentsMargins(10, 10, 10, 10);
        popupLayout->addWidget(none);
    }
    else
    {
        QWidget *row = new QWidget();
        QHBoxLayout *rl = new QHBoxLayout(row);
        rl->setContentsMargins(4, 4, 4, 4);
        rl->setSpacing(10);

        QPushButton *av = createAvatar(text, 40);
        QPushButton *nameBtn = new QPushButton(text);
        nameBtn->setObjectName("resultName");
        nameBtn->setCursor(Qt::PointingHandCursor);

        rl->addWidget(av);
        rl->addWidget(nameBtn, 1);

        connect(av, &QPushButton::clicked, this, [this, uname]()
        {
            showProfile(uname);
        });
        connect(nameBtn, &QPushButton::clicked, this, [this, uname]()
        {
            showProfile(uname);
        });

        if (uname == loggedInUser)
        {
            QLabel *you = new QLabel("You");
            you->setObjectName("muted");
            rl->addWidget(you);
        }
        else
        {
            rl->addWidget(createRelationWidget(uname, "Add"));
        }

        popupLayout->addWidget(row);
    }


    QWidget *host = searchPopup->parentWidget();
    QPoint pos = searchBox->mapTo(host, QPoint(0, searchBox->height() + 8));
    searchPopup->setFixedWidth(searchBox->width());
    searchPopup->adjustSize();
    searchPopup->move(pos);
    searchPopup->show();
    searchPopup->raise();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (searchBox && obj == searchBox)
    {
        if (event->type() == QEvent::FocusOut)
        {
            QTimer::singleShot(220, this, [this]()
            {
                if (searchPopup && searchBox && !searchBox->hasFocus())
                    searchPopup->hide();
            });
        }
        else if (event->type() == QEvent::FocusIn)
        {
            if (!searchBox->text().trimmed().isEmpty())
                runSearch();
        }
    }
    return QMainWindow::eventFilter(obj, event);
}


bool MainWindow::isFriend(const std::string &name)
{
    std::vector<std::string> f = graph.getFriends(loggedInUser);
    return std::find(f.begin(), f.end(), name) != f.end();
}

int MainWindow::mutualCount(const std::string &name)
{
    std::vector<std::string> a = graph.getFriends(loggedInUser);
    std::vector<std::string> b = graph.getFriends(name);

    int count = 0;
    for (const std::string &x : a)
        if (std::find(b.begin(), b.end(), x) != b.end())
            ++count;
    return count;
}
