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
    this->resize(800, 480); // 扩大窗口尺寸，适配配置和表格

    // 查询数据库
    UserDbHelper userDbHelper;
    QHash<QString, QString> userInfoHash = userDbHelper.getUserInfoByUUID(loginUserPhone);
    m_userInfo.userPhone = userInfoHash["phone"];
    m_userInfo.userRole = userInfoHash["role_name"];
    m_userInfo.UUID = userInfoHash["id"].toInt();
    LOG_DEBUG("主窗口模块", "当前用户ID：" << m_userInfo.UUID);

    // 初始化核心模块
    m_testTableModel = new TestTableModel(this);
    m_configWidget = new ConfigWidget(m_testTableModel, m_userInfo.UUID, this);

    // 初始化所有布局
    initAllLayout();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initAllLayout()
{
    // 初始化菜单
    initMenuBar();

    // 初始工具栏
    initToolBar();

    // 初始化状态栏
    initStatusBar();

    // 锚柳
    // initDockWidget();

    // 中心区域（核心修改：替换页面2和3）
    initCenterWidget();

    // 初始化右上角个人中心 + 渲染用户信息
    // 关键：必须传入【手机号+主窗口UI+主窗口自身指针】，缺一不可！
    m_personCenter = new PersonCenterWidget(m_userInfo.userPhone, m_userInfo.userRole, ui, this, this);
    m_personCenter->init();
}

void MainWindow::initMenuBar()
{
    // 创建菜单栏
    QMenuBar *menuBar = this->menuBar();// 添加了UI文件的话，使用这个不会导致内存泄漏
    this->setMenuBar(menuBar);

    //创建菜单
    QMenu *menu1 = new QMenu("文件", menuBar);
    QMenu *menu2 = new QMenu("编辑",menuBar);
    QMenu *menu3 = new QMenu("构建",menuBar);
    QMenu *menu4 = new QMenu("调试",menuBar);
    QMenu *menu5 = new QMenu("Analyze",menuBar);
    QMenu *menu6 = new QMenu("工具",menuBar);
    QMenu *menu7 = new QMenu("控件",menuBar);
    QMenu *menu8 = new QMenu("帮助",menuBar);

    //菜单栏添加菜单
    menuBar->addMenu(menu1);
    menuBar->addMenu(menu2);
    menuBar->addMenu(menu3);
    menuBar->addMenu(menu4);
    menuBar->addMenu(menu5);
    menuBar->addMenu(menu6);
    menuBar->addMenu(menu7);
    menuBar->addMenu(menu8);

    // 创建菜单项
    QAction *menu1Action1 = new QAction(QIcon(":/menu/static/icon/createfile.svg"), "新建文件或项目", this);
    QAction *menu1Action2 = new QAction(QIcon(":/menu/static/icon/openfile.svg"),"打开文件或项目",this);//有图标
    QAction *menu1Action3 = new QAction("退出",this);

    //菜单添加菜单项
    menu1->addAction(menu1Action1);
    menu1->addAction(menu1Action2);
    menu1->addSeparator();//插入分割线

    // 菜单里添加次级菜单
    QMenu *menu1_2 = new QMenu("最近访问的文件",this);
    menu1->addMenu(menu1_2);//添加二级菜单
    menu1_2->addAction(new QAction("暂无最近打开项目",this));//二级菜单添加菜单项

    menu1->addAction(menu1Action3);

    //菜单项添加快捷键
    menu1Action1->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));//快捷键ctrl+N
    menu1Action2->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));//快捷键ctrl+O

    connect(menu1Action1, SIGNAL(triggered(bool)), this, SLOT(createFile()));//连接信号槽
}

void MainWindow::initToolBar()
{
    QToolBar *toolBar = new QToolBar(this);
    this->addToolBar(Qt::LeftToolBarArea, toolBar);

    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);//设置工具项为图片在文字上方

    //创建工具项
    QAction *action4 = new QAction(QIcon(":/menu/static/icon/welcome.svg"), "欢迎", this);
    QAction *action5 = new QAction(QIcon(":/menu/static/icon/algorithms5.svg"), "算法测试", this); // 改为“算法测试”
    QAction *action6 = new QAction(QIcon(":/menu/static/icon/data-view.svg"), "结果保存", this); // 改为“结果保存”
    QAction *action7 = new QAction(QIcon(":/menu/static/icon/AIAssistance.svg"), "AI助手", this);
    QAction *action8 = new QAction(QIcon(":/menu/static/icon/usermanager.svg"), "用户信息", this);
    QAction *action9 = new QAction(QIcon(":/menu/static/icon/系统监控.svg"), "系统监控", this);

    //工具栏添加工具项
    toolBar->addAction(action4);
    toolBar->addSeparator();//添加分割线
    toolBar->addAction(action5);
    toolBar->addSeparator();//添加分割线
    toolBar->addAction(action6);
    toolBar->addSeparator();//添加分割线
    toolBar->addAction(action7);
    toolBar->addSeparator();//添加分割线
    toolBar->addAction(action8);
    toolBar->addSeparator();//添加分割线
    toolBar->addAction(action9);
    toolBar->addSeparator();//添加分割线

    // 工具项连接槽函数
    connect(action4, &QAction::triggered, this, &MainWindow::showWelcomePage);
    connect(action5, &QAction::triggered, this, &MainWindow::showEditPage);    // 算法测试页面（原编辑页面）
    connect(action6, &QAction::triggered, this, &MainWindow::showDesignPage);  // 结果保存页面（原设计页面）
    connect(action7, &QAction::triggered, this, &MainWindow::showHelpPage);
    connect(action8, &QAction::triggered, this, &MainWindow::showUserManagerPage);
    connect(action9, &QAction::triggered, this, &MainWindow::showLogPage);
}

void MainWindow::initStatusBar()
{
    //创建状态栏
    QStatusBar *statusBar = new QStatusBar(this);

    statusBar->setObjectName("状态栏");
    statusBar->setStyleSheet("QStatusBar::item{border: 0px}"); //设置不显示label的边框

    //主窗口添加状态栏
    this->setStatusBar(statusBar);

    //创建标签
    QLabel *statusLabel = new QLabel("系统就绪", this);
    QLabel *statusLabel2 = new QLabel("当前用户：" + m_userInfo.userPhone, this);

    //状态栏添加信息
    statusBar->showMessage("初始化完成", 3000);//显示在左侧，并且3秒后自动消失
    statusBar->addWidget(statusLabel2, 100);
    statusBar->addPermanentWidget(statusLabel);//添加右侧标签(永久性)
}

void MainWindow::initDockWidget()
{
    //创建铆接部件
    QDockWidget *dockWidget = new QDockWidget(this);
    dockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);//设置铆接部件不可移动、不可关闭、不可浮动
    dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);//设置允许左右停靠

    QLabel *titleLabel = new QLabel("铆接");
    titleLabel->setAlignment(Qt::AlignCenter);//设置中心对齐
    dockWidget->setTitleBarWidget(titleLabel);//设置标题栏

    //主窗口添加铆接部件
    this->addDockWidget(Qt::LeftDockWidgetArea,dockWidget);//设置铆接部件停靠在左侧

    //创建ListWidget
    QListWidget *textList = new QListWidget(this);
    for(int i=1; i<=50; i++)
    {
        QListWidgetItem *item = new QListWidgetItem(QString("%1").arg(i));
        textList->addItem(item);
        item->setTextAlignment(Qt::AlignRight);
    }
    textList->setFixedWidth(50);
    textList->setObjectName("文本列表");
    //设置没有水平以及垂直滑动条
    textList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //铆接部件添加内容主体
    dockWidget->setWidget(textList);
}

void MainWindow::initCenterWidget()
{
    // 1. 创建堆叠窗口，作为主窗口唯一的中心部件
    m_stackedWidget = new QStackedWidget(this);
    this->setCentralWidget(m_stackedWidget);

    // 2. ========== 页面0：欢迎页面 ==========
    QWidget *welcomeWidget = new QWidget(this);
    QVBoxLayout *welcomeLayout = new QVBoxLayout(welcomeWidget);
    QLabel *welcomeLabel = new QLabel("🎉 基于自动合成李雅普诺夫函数的控制器算法研究分析平台", this);
    welcomeLabel->setStyleSheet("font-size:24px;color:#2196F3;font-weight:bold;");
    welcomeLabel->setAlignment(Qt::AlignCenter);
    QLabel *welcomeSubLabel = new QLabel("请通过左侧工具栏选择功能模块", this);
    welcomeSubLabel->setStyleSheet("font-size:16px;color:#666;");
    welcomeSubLabel->setAlignment(Qt::AlignCenter);
    welcomeLayout->addStretch();
    welcomeLayout->addWidget(welcomeLabel);
    welcomeLayout->addSpacing(20);
    welcomeLayout->addWidget(welcomeSubLabel);
    welcomeLayout->addStretch();
    welcomeWidget->setLayout(welcomeLayout);
    m_stackedWidget->addWidget(welcomeWidget); // 索引0

    // 3. ========== 页面1：算法测试页面 ==========
    m_stackedWidget->addWidget(m_configWidget); // 索引1

    // 4. ========== 页面2：结果保存页面 ==========
    QWidget *resultWidget = m_configWidget->getTestTableWidget();
    m_stackedWidget->addWidget(resultWidget); // 索引2

    // 5. ========== 页面3：AI助手页面 ==========
//    int currentUserId = 1; // 示例：假设当前用户ID为1
    LLMWidget *llmWidget = new LLMWidget(m_userInfo.UUID, this);
    m_stackedWidget->addWidget(llmWidget); // 索引3

    // 6. ========== 页面4：用户信息表页面 ==========
    initCenterWidgetUserTable();

    // 7. ========== 页面5：系统监控页面 ==========
    initCenterWidgetLogTable();

    // 8. 默认显示：欢迎页面
    m_stackedWidget->setCurrentIndex(0);
}

void MainWindow::initCenterWidgetUserTable()
{
    // 创建表格模块并添加到栈控件中
    TableOperateWidget *userTable = new UserTableWidget(this);
    m_stackedWidget->addWidget(userTable); // 索引4
}

void MainWindow::initCenterWidgetLogTable()
{
    // 创建表格模块并添加到栈控件中
    LogTableWidget *logTableWidget = new LogTableWidget(m_userInfo.UUID, this);
    m_stackedWidget->addWidget(logTableWidget); // 索引5
    logTableWidget->loadTableData();
}


// ========== 页面切换槽函数 ==========
void MainWindow::createFile()
{
    // 预留空函数（原菜单新建文件）
}

void MainWindow::showWelcomePage()
{
    m_stackedWidget->setCurrentIndex(0); // 索引0：欢迎页面
    qDebug() << "切换到：欢迎页面";
}

void MainWindow::showEditPage()
{
    m_stackedWidget->setCurrentIndex(1); // 索引1：算法测试页面
    qDebug() << "切换到：算法测试页面";
}

void MainWindow::showDesignPage()
{
    m_stackedWidget->setCurrentIndex(2); // 索引2：结果保存页面
    qDebug() << "切换到：结果保存页面";
}

void MainWindow::showHelpPage()
{
    m_stackedWidget->setCurrentIndex(3); // 索引3：AI助手页面
    qDebug() << "切换到：AI助手页面";
}

void MainWindow::showUserManagerPage()
{
    m_stackedWidget->setCurrentIndex(4); // 索引4：用户信息页面
    qDebug() << "切换到：用户信息页面";
}

void MainWindow::showLogPage()
{
    m_stackedWidget->setCurrentIndex(5); // 索引5：系统监控页面
    qDebug() << "切换到：系统监控页面" ;
}
