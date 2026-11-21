#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QWidget>
#include <QLineEdit>
#include <QFrame>
#include <QVBoxLayout>
#include <QPushButton>
#include <QEvent>
#include <string>
#include <vector>

#include "auth.h"
#include "SocialGraph.h"
#include "suggestion.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::MainWindow *ui;

    SocialGraph graph;
    UserSystem userSystem;
    FriendSuggestion suggestion;

    std::string loggedInUser;
    std::string currentProfileUser;

    QStackedWidget *appPages;
    QStackedWidget *dashboardPages;

    QWidget *loginPage;
    QWidget *dashboardPage;

    QWidget *homePageWidget = nullptr;
    QWidget *profilePageWidget = nullptr;
    QVBoxLayout *homeLayout = nullptr;
    QVBoxLayout *profileLayout = nullptr;
    QLineEdit *searchBox = nullptr;
    QFrame *searchPopup = nullptr;
    QVBoxLayout *popupLayout = nullptr;
    QPushButton *bellButton = nullptr;
    QPushButton *chatButton = nullptr;

    std::vector<std::string> incomingRequests;   
    std::vector<std::string> outgoingRequests;   

    void initializeDatabase();
    void handleLogin();
    void handleRegistration();
    void logout();

    void showDashboard();
    QWidget *createDashboard();
    QWidget *createTopBar(QWidget *dashboard);
    QWidget *createScrollPage(QVBoxLayout *&contentLayout);

    void showHome();
    void showProfile(const std::string &user);
    void showChatDialog();
    void populateHome();
    void populateProfile();

    void runSearch();

    void sendRequest(const std::string &name);
    void acceptRequest(const std::string &name);
    void rejectRequest(const std::string &name);
    void cancelRequest(const std::string &name);
    void removeFriend(const std::string &name);
    void loadRequests();
    void refreshBell();
    void pollServer();
    QWidget *createRelationWidget(const std::string &user,
                                  const QString &addText);
    void afterFriendshipChanged();
    bool isFriend(const std::string &name);
    int  mutualCount(const std::string &name);

    QPushButton *createAvatar(const QString &name, int size);
    void clearLayout(QLayout *layout);
};
#endif
