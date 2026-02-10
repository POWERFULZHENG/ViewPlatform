#include "personcenterwidget.h"
#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "userdbhelper.h"
#include "loginwidget.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QTimer>
#include <QEvent>
#include <QPixmap>
#include <QIcon>
#include <QPainter>
#include "loghelper.h"


// ===================== 构造函数【初始化列表顺序】 =====================
PersonCenterWidget::PersonCenterWidget(const QString& personPhone, const QString& role, Ui::MainWindow* ui, QMainWindow *mainWindow, QWidget *parent)
    : QWidget(parent)
    , m_loginPersonPhone(personPhone)
    , m_loginPersonRole(role)
    , m_ui(ui)
    , m_mainWindow(mainWindow)
    , m_isHovering(false)
    , m_personBtn(nullptr)
    , m_personCenterWidget(nullptr)
    , m_headLabel(nullptr)
    , m_phoneLabel(nullptr)
    , m_roleLabel(nullptr)
    , m_switchBtn(nullptr)
{

}

PersonCenterWidget::~PersonCenterWidget()
{
}

void PersonCenterWidget::init()
{
    initPersonCenter();
    renderPersonInfo();
}

void PersonCenterWidget::initPersonCenter()
{
    // ========== 1. 创建右上角【个人中心按钮】- 菜单栏最右侧 ==========
    m_personBtn = new QToolButton(m_mainWindow);
    m_personBtn->setIconSize(QSize(16, 16));
    // 优化：使用QT内置样式图标（更稳定，无资源文件依赖），备选自定义图标
    // 恢复Qt5.14.2支持的代码，补充默认图标兜底（防止主题图标不存在）
    m_personBtn->setIcon(QIcon::fromTheme("user", QIcon(":/menu/static/icon/default_user.png")));
    m_personBtn->setFixedSize(70, 25);

    // 优化1：样式表优化（悬浮有明显视觉变化、去掉默认背景、圆角更柔和）
    m_personBtn->setStyleSheet(R"(
        QToolButton {
            border: none;
            font-size: 12px;
            color: #333333;
            padding: 0 8px;
            background-color: transparent; /* 默认透明，更美观 */
        }
        QToolButton:hover {
            background-color: #E8F4FF; /* 浅蓝悬浮背景，视觉更友好 */
            border-radius: 6px;
            color: #1E90FF; /* 字体变色，强化悬浮反馈 */
        }
    )");

    m_personBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_personBtn->setText("个人中心");

    // 把按钮放到菜单栏最右侧（固定位置）
    m_ui->menubar->setCornerWidget(m_personBtn, Qt::TopRightCorner);
    m_personBtn->raise();   // 置顶按钮，防止被遮挡
    m_personBtn->show();    // 强制显示

    // ========== 2. 创建【个人中心面板】- 核心：自定义QWidget，绝对不超出主界面 ==========
    m_personCenterWidget = new QWidget(m_mainWindow);
    m_personCenterWidget->setFixedSize(230, 260); // 优化：适当增高面板，避免控件拥挤
    // 优化2：面板样式优化（默认边框柔和，悬浮边框高亮，去掉hover时的背景变化）
    m_personCenterWidget->setStyleSheet(R"(
        QWidget {
            background-color: white;
            border: 1px solid #E5E5E5;
            border-radius: 8px;
            padding: 5px;
        }
        QWidget:hover {
            border: 1px solid #1E90FF; /* 悬浮高亮边框，保留你的逻辑 */
        }
    )");
    m_personCenterWidget->setVisible(false); // 默认隐藏，鼠标悬浮再显示
    m_personCenterWidget->setAttribute(Qt::WA_TransparentForMouseEvents, false); // 禁止鼠标穿透

    // ========== 3. 面板内布局：头像 + 手机号 + role + 切换用户按钮 ==========
    QVBoxLayout *vLayout = new QVBoxLayout(m_personCenterWidget);
    vLayout->setContentsMargins(20, 15, 20, 15);
    vLayout->setSpacing(18); // 优化3：减小间距，避免控件过于分散，布局更紧凑
    vLayout->setAlignment(Qt::AlignTop); // 优化：控件顶部对齐，整体更规整

    // 3.1 圆形头像（优化：图片和文字状态都保证完美圆形，样式统一）
    m_headLabel = new QLabel(m_personCenterWidget);
    m_headLabel->setFixedSize(70, 70);
    m_headLabel->setAlignment(Qt::AlignCenter); // 文字/图片居中

    // ====== 修改这里为你的头像路径，无图片则显示默认文字头像 ======
    QPixmap pixmap(":/menu/static/icon/Docker.svg");
    if(pixmap.isNull())
    {
        m_headLabel->setStyleSheet(R"(
            QLabel {
                border: 2px solid #1E90FF;
                border-radius: 35px; /* 优化：70px宽高，圆角35px才是完美圆形 */
                background: #F8F8FF;
                font-size: 16px;
                color: #1E90FF;
            }
        )");
        m_headLabel->setText("用户");
    }
    else
    {
        // 优化4：图片裁剪为圆形，避免方形图片破坏视觉效果
        QPixmap roundedPixmap = QPixmap(m_headLabel->size());
        roundedPixmap.fill(Qt::transparent); // 透明背景
        QPainter painter(&roundedPixmap);
        painter.setRenderHint(QPainter::Antialiasing); // 抗锯齿，圆角更平滑
        // 绘制圆形裁剪区域
        painter.setClipRegion(QRegion(QRect(0, 0, 70, 70), QRegion::Ellipse));
        painter.drawPixmap(0, 0, pixmap.scaled(m_headLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        painter.end();

        m_headLabel->setPixmap(roundedPixmap);
        m_headLabel->setStyleSheet(R"(
            QLabel {
                border-radius: 35px; /* 完美圆形 */
                border: 2px solid #FFFFFF; /* 白色边框，提升质感 */
            }
        )");
    }

    // 3.2 登录手机号（数据库查询）- 优化：样式更简洁，添加占位符
    m_phoneLabel = new QLabel(m_personCenterWidget);
    m_phoneLabel->setStyleSheet(R"(
        QLabel {
            font-size: 15px;
            color: #333333;
            height: 20px; /* 优化：减小高度，避免冗余留白 */
        }
    )");
    m_phoneLabel->setAlignment(Qt::AlignCenter);
    m_phoneLabel->setText("138****1234"); // 占位符，后续从数据库更新

    // 3.3 角色标签 - 优化：样式区分手机号，添加浅背景突出角色
    m_roleLabel = new QLabel(m_personCenterWidget);
    m_roleLabel->setStyleSheet(R"(
        QLabel {
            font-size: 13px;
            color: #1E90FF;
            background-color: #E8F4FF;
            border-radius: 10px;
            padding: 2px 12px;
            height: 24px; /* 优化：适配圆角高度 */
        }
    )");
    m_roleLabel->setAlignment(Qt::AlignCenter);
    m_roleLabel->setText("普通用户"); // 占位符，后续从数据库更新

    // 3.4 切换用户按钮 - 优化：按钮宽度适配面板，视觉更协调
    m_switchBtn = new QPushButton("切换用户", m_personCenterWidget);
    m_switchBtn->setFixedSize(190, 36); // 优化：固定宽度（面板宽度-左右内边距），更规整
    m_switchBtn->setStyleSheet(R"(
        QPushButton {
            background: #1E90FF;
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 14px;
        }
        QPushButton:hover {
            background: #4169E1; /* 深蓝悬浮，保留你的逻辑 */
        }
    )");
    connect(m_switchBtn, &QPushButton::clicked, this, &PersonCenterWidget::slot_switchPerson);

    // 优化5：添加布局拉伸因子，让切换按钮底部对齐，避免面板下方留白
    QSpacerItem *verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    // 添加控件到面板布局（按顺序：头像 → 手机号 → 角色 → 拉伸控件 → 切换按钮）
    vLayout->addWidget(m_headLabel, 0, Qt::AlignCenter);
    vLayout->addWidget(m_phoneLabel, 0, Qt::AlignCenter);
    vLayout->addWidget(m_roleLabel, 0, Qt::AlignCenter);
    vLayout->addSpacerItem(verticalSpacer); // 拉伸控件，推动切换按钮到底部
    vLayout->addWidget(m_switchBtn, 0, Qt::AlignCenter);

    // ========== 给按钮和面板 安装事件过滤器 ==========
    m_personBtn->installEventFilter(this);
    m_personCenterWidget->installEventFilter(this);
}

// ===================== 查询数据库，渲染用户手机号信息 =====================
void PersonCenterWidget::renderPersonInfo()
{
    if(this->m_loginPersonPhone.isEmpty())
    {
        m_phoneLabel->setText("未登录");
        m_phoneLabel->setStyleSheet("QLabel{font-size:15px; color:#999999; padding-left:5px;}");
        return;
    }
    LOG_DEBUG("个人中心模块", "用户手机号：" << m_loginPersonPhone) ;
    // 调用数据库工具类，查询用户信息
    UserDbHelper userDbHelper;
    QString personPhone = userDbHelper.getUserInfoByPhone(m_loginPersonPhone);
    if(!personPhone.isEmpty())
    {
        m_phoneLabel->setText("当前登录：" + m_loginPersonPhone);
        m_roleLabel->setText("用户角色：" + m_loginPersonRole);
    }
    else
    {
        m_phoneLabel->setText("用户信息异常");
        m_phoneLabel->setStyleSheet("QLabel{font-size:15px; color:#FF4500; padding-left:5px;}");
        m_roleLabel->setText("用户信息异常");
        m_roleLabel->setStyleSheet("QLabel{font-size:15px; color:#FF4500; padding-left:5px;}");
    }
}

// ===================== 事件过滤器，完美监听鼠标悬停/离开 =====================
bool PersonCenterWidget::eventFilter(QObject *watched, QEvent *event)
{
    // 修复3：前置精准过滤，只处理【按钮/面板】的事件，其他控件事件直接返回，不处理不打印
    if(watched != m_personBtn && watched != m_personCenterWidget)
    {
        return QWidget::eventFilter(watched, event);
    }
    // 修复4：只监听【WA_Hover专属事件】，删除重复的Enter/Leave，彻底杜绝重复触发
    // 开启WA_Hover后，控件只会触发 HoverEnter/HoverLeave，不会重复触发，这是QT的标准用法
    if (event->type() == QEvent::Enter)
    {
        m_isHovering = true;
        slot_showPersonCenter();
        LOG_DEBUG("个人中心模块", "正常悬浮触发");
        return true; // 我们处理了这个事件
    }
    else if (event->type() == QEvent::Leave)
    {
        m_isHovering = false;
        // 延迟150ms隐藏，足够鼠标从按钮移到面板，无缝衔接
        QTimer::singleShot(200, this, [=](){
            if(!m_isHovering) slot_hidePersonCenter();
        });
        LOG_DEBUG("个人中心模块", "正常离开触发") ;
        return true; // 我们处理了这个事件
    }
    // 其他事件（例如鼠标点击、绘制、焦点等等）不要吞掉，交给基类默认处理
    return QWidget::eventFilter(watched, event);
}

// ===================== 核心槽函数：切换用户 =====================
void PersonCenterWidget::slot_switchPerson()
{
    int ret = QMessageBox::question(this, "提示", "确定要切换用户吗？", QMessageBox::Yes | QMessageBox::No);
    if(ret == QMessageBox::Yes)
    {
        // 1. 关闭当前主界面
        slot_hidePersonCenter(); // 先隐藏面板
        m_mainWindow->close();    // 修复：关闭【主窗口】而不是当前控件！！！
        // 2. 重新打开登录窗口
        LoginWidget loginWidget;
        // 3. 登录成功则重新打开主界面
        if(loginWidget.exec() == QDialog::Accepted || loginWidget.isLoginSuccess())
        {
            MainWindow *newMainWindow = new MainWindow(loginWidget.m_curLoginPhone);
            newMainWindow->setWindowTitle("基于自动合成李雅普诺夫函数的控制器算法研究分析平台");
            newMainWindow->show();
        }
    }
    // 切换用户后隐藏面板
    slot_hidePersonCenter();
}

// ===================== 核心槽函数：显示个人中心面板【精准定位，不超界面】 =====================
void PersonCenterWidget::slot_showPersonCenter()
{
    if(m_personCenterWidget->isVisible()) return;
    // 坐标基于【主窗口】计算，父对象是主窗口，坐标100%正确，绝对不超界
    int panelX = m_mainWindow->width() - m_personCenterWidget->width() - 15;  // 右内缩15px，更美观
    int panelY = m_ui->menubar->height() + 3;                               // 基于菜单栏高度，紧贴菜单栏下方，无偏移
    m_personCenterWidget->move(panelX, panelY);
    m_personCenterWidget->raise(); // 每次显示都置顶，防止被遮挡
    m_personCenterWidget->show();  // 用show()替代setVisible，强制显示，QT更推荐
}

// ===================== 核心槽函数：隐藏个人中心面板 =====================
void PersonCenterWidget::slot_hidePersonCenter()
{
    // 双重判断：面板可见 + 非悬停状态，才隐藏，杜绝无效调用
    if(m_isHovering || !m_personCenterWidget->isVisible())
    {
        return;
    }
    m_personCenterWidget->setVisible(false);
}
