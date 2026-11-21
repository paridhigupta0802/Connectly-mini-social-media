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
