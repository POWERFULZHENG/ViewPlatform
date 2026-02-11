#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "usertablewidget.h"
#include "logtablewidget.h"
#include "ConfigWidget.h"
#include "TestTableModel.h"
#include "PythonRunner.h"
#include "TestDbHelper.h"
#include "userdbhelper.h"
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QIcon>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QDockWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QGroupBox>
#include "loghelper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableView>
#include <QHeaderView>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QDialog>
#include <QJsonDocument>
#include <QScrollArea>
#include <QHash>

MainWindow::MainWindow(QString loginUserPhone, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("基于自动合成李雅普诺夫函数的控制器算法研究分析平台");
    this->setWindowIcon(QIcon(":/menu/static/icon/Docker.svg"));
    this->resize(800, 480);

    // ========== 核心从数据库加载用户信息并设置到会话模块 ==========
    UserDbHelper userDbHelper;
    QHash<QString, QString> userInfoHash = userDbHelper.getUserInfoByUUID(loginUserPhone);

    // 构建完整的用户信息结构体（对应sys_user表）
    UserInfo userInfo;
    userInfo.id = userInfoHash["id"].toInt();
    userInfo.userName = userInfoHash["user_name"];
    userInfo.nickName = userInfoHash["nick_name"];
    userInfo.roleName = userInfoHash["role_name"];
    userInfo.phone = userInfoHash["phone"];
    userInfo.status = userInfoHash["status"].toInt();
    userInfo.createTime = QDateTime::fromString(userInfoHash["create_time"], "yyyy-MM-dd hh:mm:ss");

    // 设置到全局会话模块（替代原m_userInfo）
    UserSession::instance()->setCurrentUser(userInfo);

    // 日志输出（使用会话模块的信息）
    LOG_DEBUG("主窗口模块", "当前用户ID：" << UserSession::instance()->userId()
              << "，手机号：" << UserSession::instance()->userPhone()
              << "，角色：" << UserSession::instance()->userRole());

    // 初始化核心模块（使用会话模块的用户ID）
    m_testTableModel = new TestTableModel(this);
    m_configWidget = new ConfigWidget(m_testTableModel, this);

    // 初始化所有布局
    initAllLayout();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ========== 权限判断：改用会话模块的方法 ==========
bool MainWindow::isAdmin() const
{
    return UserSession::instance()->isAdmin();
}

void MainWindow::initAllLayout()
{
    initMenuBar();
    initToolBar();
    initStatusBar();
    initCenterWidget();

    // 初始化个人中心（改用会话模块的信息）
    m_personCenter = new PersonCenterWidget(
        UserSession::instance()->userPhone(),
        UserSession::instance()->userRole(),
        ui, this, this
    );
    m_personCenter->init();
}

void MainWindow::initMenuBar()
{
//    QMenuBar *menuBar = this->menuBar();
//    this->setMenuBar(menuBar);

//    QMenu *menu1 = new QMenu("文件", menuBar);
//    QMenu *menu2 = new QMenu("编辑",menuBar);
//    QMenu *menu3 = new QMenu("构建",menuBar);
//    QMenu *menu4 = new QMenu("调试",menuBar);
//    QMenu *menu5 = new QMenu("Analyze",menuBar);
//    QMenu *menu6 = new QMenu("工具",menuBar);
//    QMenu *menu7 = new QMenu("控件",menuBar);
//    QMenu *menu8 = new QMenu("帮助",menuBar);

//    // ========== 可选：普通用户隐藏"调试"菜单（示例） ==========
//    if (!isAdmin()) {
//        menu4->setVisible(false);
//    }

//    menuBar->addMenu(menu1);
//    menuBar->addMenu(menu2);
//    menuBar->addMenu(menu3);
//    menuBar->addMenu(menu4);
//    menuBar->addMenu(menu5);
//    menuBar->addMenu(menu6);
//    menuBar->addMenu(menu7);
//    menuBar->addMenu(menu8);

//    // 创建菜单项
//    QAction *menu1Action1 = new QAction(QIcon(":/menu/static/icon/createfile.svg"), "新建文件或项目", this);
//    QAction *menu1Action2 = new QAction(QIcon(":/menu/static/icon/openfile.svg"),"打开文件或项目",this);
//    QAction *menu1Action3 = new QAction("退出",this);

//    menu1->addAction(menu1Action1);
//    menu1->addAction(menu1Action2);
//    menu1->addSeparator();
//    QMenu *menu1_2 = new QMenu("最近访问的文件",this);
//    menu1->addMenu(menu1_2);
//    menu1_2->addAction(new QAction("暂无最近打开项目",this));
//    menu1->addAction(menu1Action3);

//    menu1Action1->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
//    menu1Action2->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
//    connect(menu1Action1, SIGNAL(triggered(bool)), this, SLOT(createFile()));
}

void MainWindow::initToolBar()
{
    QToolBar *toolBar = new QToolBar(this);
    this->addToolBar(Qt::LeftToolBarArea, toolBar);

    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    // ========== 所有用户可见的工具项 ==========
    QAction *action4 = new QAction(QIcon(":/menu/static/icon/welcome.svg"), "欢迎", this);
    QAction *action5 = new QAction(QIcon(":/menu/static/icon/algorithms5.svg"), "算法测试", this);
    QAction *action6 = new QAction(QIcon(":/menu/static/icon/data-view.svg"), "结果保存", this);
    QAction *action7 = new QAction(QIcon(":/menu/static/icon/AIAssistance.svg"), "AI助手", this);

    // 添加公共工具项
    toolBar->addAction(action4);
    toolBar->addSeparator();
    toolBar->addAction(action5);
    toolBar->addSeparator();
    toolBar->addAction(action6);
    toolBar->addSeparator();
    toolBar->addAction(action7);
    toolBar->addSeparator();

    QAction *action8 = new QAction(QIcon(":/menu/static/icon/系统监控.svg"), "系统监控", this);
    toolBar->addAction(action8);
    toolBar->addSeparator();
    connect(action8, &QAction::triggered, this, &MainWindow::showLogPage);

    // ========== 核心仅管理员添加专属工具项 ==========
    if (isAdmin()) {
        QAction *action9 = new QAction(QIcon(":/menu/static/icon/usermanager.svg"), "用户信息", this);
        // 管理员专属工具项添加+连接槽函数
        toolBar->addAction(action9);
        toolBar->addSeparator();

        connect(action9, &QAction::triggered, this, &MainWindow::showUserManagerPage);
    }

    // 公共工具项连接槽函数
    connect(action4, &QAction::triggered, this, &MainWindow::showWelcomePage);
    connect(action5, &QAction::triggered, this, &MainWindow::showEditPage);
    connect(action6, &QAction::triggered, this, &MainWindow::showDesignPage);
    connect(action7, &QAction::triggered, this, &MainWindow::showHelpPage);
}

void MainWindow::initStatusBar()
{
    QStatusBar *statusBar = new QStatusBar(this);
    statusBar->setObjectName("状态栏");
    statusBar->setStyleSheet("QStatusBar::item{border: 0px}");
    this->setStatusBar(statusBar);

    QLabel *statusLabel = new QLabel("系统就绪", this);
    // ========== 核心从会话模块获取用户信息 ==========
    QString userInfoText = QString("当前用户：%1（%2 / %3）")
                           .arg(UserSession::instance()->userPhone())
                           .arg(UserSession::instance()->userNickName())
                           .arg(UserSession::instance()->userRole());
    QLabel *statusLabel2 = new QLabel(userInfoText, this);

    statusBar->showMessage("初始化完成", 3000);
    statusBar->addWidget(statusLabel2, 100);
    statusBar->addPermanentWidget(statusLabel);
}

void MainWindow::initCenterWidget()
{
    m_stackedWidget = new QStackedWidget(this);
    this->setCentralWidget(m_stackedWidget);

    // 页面0：欢迎页面（所有用户）
    QWidget *welcomeWidget = new QWidget(this);
    QVBoxLayout *welcomeLayout = new QVBoxLayout(welcomeWidget);
    QLabel *welcomeLabel = new QLabel("基于自动合成李雅普诺夫函数的控制器算法研究分析平台", this);
    welcomeLabel->setStyleSheet("font-size:24px;color:#2196F3;font-weight:bold;");
    welcomeLabel->setAlignment(Qt::AlignCenter);
    // ========== 核心从会话模块获取角色信息 ==========
    QLabel *welcomeSubLabel = new QLabel(
        QString("当前角色：%1，请通过左侧工具栏选择功能模块").arg(UserSession::instance()->userRole()),
        this
    );
    welcomeSubLabel->setStyleSheet("font-size:16px;color:#666;");
    welcomeSubLabel->setAlignment(Qt::AlignCenter);
    welcomeLayout->addStretch();
    welcomeLayout->addWidget(welcomeLabel);
    welcomeLayout->addSpacing(20);
    welcomeLayout->addWidget(welcomeSubLabel);
    welcomeLayout->addStretch();
    welcomeWidget->setLayout(welcomeLayout);
    m_stackedWidget->addWidget(welcomeWidget); // 索引0

    // 页面1：算法测试页面（所有用户）
    m_stackedWidget->addWidget(m_configWidget); // 索引1

    // 页面2：结果保存页面（所有用户）
    QWidget *resultWidget = m_configWidget->getTestTableWidget();
    m_stackedWidget->addWidget(resultWidget); // 索引2

    // 页面3：AI助手页面（所有用户）
    // ========== 核心从会话模块获取用户ID ==========
    LLMWidget *llmWidget = new LLMWidget(UserSession::instance()->userId(), this);
    m_stackedWidget->addWidget(llmWidget); // 索引3

    // 页面4：系统监控页面
    // ========== 核心从会话模块获取用户ID ==========
    LogTableWidget *logTableWidget = new LogTableWidget(this);
    m_stackedWidget->addWidget(logTableWidget); // 索引4
    logTableWidget->loadTableData();

    // ========== 仅管理员创建专属页面（普通用户无入口，无需占位） ==========
    if (isAdmin()) {
        // 页面5：用户信息表页面
        TableOperateWidget *userTable = new UserTableWidget(this);
        m_stackedWidget->addWidget(userTable); // 索引5
    }

    // 默认显示欢迎页面
    m_stackedWidget->setCurrentIndex(0);
}

// ========== 页面切换槽函数 ==========
void MainWindow::createFile()
{
    // 可选：普通用户禁用新建
    if (!isAdmin()) {
        QMessageBox::warning(this, "权限不足", "普通用户无新建文件权限！");
        return;
    }
    // 管理员逻辑...
}

void MainWindow::showWelcomePage()
{
    m_stackedWidget->setCurrentIndex(0);
    LOG_INFO("主窗口模块", "欢迎页面");
}

void MainWindow::showEditPage()
{
    m_stackedWidget->setCurrentIndex(1);
    LOG_INFO("主窗口模块", "算法测试页面");
}

void MainWindow::showDesignPage()
{
    m_stackedWidget->setCurrentIndex(2);
    LOG_INFO("主窗口模块", "结果保存页面");
}

void MainWindow::showHelpPage()
{
    m_stackedWidget->setCurrentIndex(3);
    LOG_INFO("页面切换", "AI助手页面");
}

void MainWindow::showLogPage()
{
    m_stackedWidget->setCurrentIndex(4);
    LOG_INFO("主窗口模块", "系统监控页面");
}

// ========== 仅管理员可调用的槽函数（普通用户无入口） ==========
void MainWindow::showUserManagerPage()
{
    if (!isAdmin()) { // 双重校验，防止非法调用
        QMessageBox::warning(this, "权限不足", "仅超级管理员可访问用户信息页面！");
        return;
    }
    m_stackedWidget->setCurrentIndex(5);
    LOG_INFO("主窗口模块", "用户信息页面");
}
