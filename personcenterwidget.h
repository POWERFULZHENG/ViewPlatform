#ifndef PERSONCENTERWIDGET_H
#define PERSONCENTERWIDGET_H

#include <QWidget>
#include <QToolButton>
#include <QLabel>
#include <QPushButton>
#include <QEvent>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class LoginWidget;
class MysqlHelper;

class PersonCenterWidget : public QWidget
{
    Q_OBJECT
public:
    // 增加主窗口指针参数，用于关闭主窗口+计算面板坐标，必须传！
    explicit PersonCenterWidget(const QString& personPhone, const QString& role, Ui::MainWindow* ui, QMainWindow *mainWindow, QWidget *parent = nullptr);
    ~PersonCenterWidget() override;
    void init(); // 初始化入口

private slots:
    void slot_switchPerson();    // 切换用户
    void slot_showPersonCenter();// 显示面板
    void slot_hidePersonCenter();// 隐藏面板

private:
    void initPersonCenter();     // 初始化面板控件
    void renderPersonInfo();     // 渲染用户信息
    // 重写事件过滤器【核心】
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QString         m_loginPersonPhone;  // 当前登录手机号
    QString         m_loginPersonRole;  // 当前登录用户角色
    Ui::MainWindow* m_ui;                // 主窗口UI指针
    QMainWindow*    m_mainWindow;        // 主窗口指针【新增：核心解决关闭主窗口】
    bool            m_isHovering;        // 悬停状态标记位
    QToolButton*    m_personBtn;         // 个人中心按钮
    QWidget*        m_personCenterWidget;// 个人中心面板
    QLabel*         m_headLabel;         // 头像
    QLabel*         m_phoneLabel;        // 手机号
    QLabel*         m_roleLabel;        // 身份
    QPushButton*    m_switchBtn;         // 切换用户按钮
};

#endif // PERSONCENTERWIDGET_H
