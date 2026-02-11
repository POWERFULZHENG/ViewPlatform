#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QToolButton>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include "personcenterwidget.h"
#include "ConfigWidget.h"
#include "TestTableModel.h"
#include "PythonRunner.h"
#include "TestDbHelper.h"
#include "ui_ConfigWidget.h"
#include "llmwidget.h"
#include "UserSession.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
namespace Ui { class ConfigWidget; }
QT_END_NAMESPACE

//struct UserInfo{
//    int UUID;
//    QString userRole;
//    QString userPhone;
//};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QString loginUserPhone, QWidget *parent = nullptr);
    ~MainWindow();
    bool isAdmin() const;
     void initAllLayout();
     void initMenuBar();
     void initToolBar();
     void initStatusBar();
     void initDockWidget();
     void initCenterWidget();
     void initCenterWidgetUserTable();
     void initCenterWidgetLogTable();

private slots:
     void createFile();
     // ========== 新增：4个页面切换的槽函数（也可以合并为1个带参数的槽函数） ==========
     void showWelcomePage();
     void showEditPage();
     void showDesignPage();
     void showHelpPage();
     void showUserManagerPage();
     void showLogPage();
private:
     Ui::MainWindow *ui;
     Ui::ConfigWidget *configUi;
     //设置中心部件
     QStackedWidget *m_stackedWidget;
     PersonCenterWidget *m_personCenter;

     // 核心模块实例
     ConfigWidget *m_configWidget;          // 算法配置控件
     TestTableModel *m_testTableModel;      // 测试结果表格模型

     UserInfo m_userInfo;
};
#endif // MAINWINDOW_H
