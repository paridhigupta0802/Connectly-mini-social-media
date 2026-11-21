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
