#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QLabel>
#include <QSlider>
#include <QTimer>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QRegExpValidator>
#include "userdbhelper.h"

// 登录窗口核心类，继承QWidget
class LoginWidget : public QDialog
{
    Q_OBJECT // QT信号槽必须的宏

public:
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget() override = default;

    // 对外提供登录成功的标识，给main函数判断是否跳转主界面
    bool isLoginSuccess() const { return m_bLoginSuccess; }
    QString m_curLoginPhone;  // ====== 新增：当前登录成功的手机号，公有变量供主界面获取 ======

private slots:
    // 1. 登录方式切换：密码登录 ↔ 验证码登录
    void slot_switchLoginMode();
    // 2. 滑块验证-松开滑块触发验证
    void slot_sliderVerify();
    // 3. 获取验证码按钮点击事件（含倒计时逻辑）
    void slot_getVerificationCode();
    // 4. 验证码倒计时槽函数
    void slot_countDownTime();
    // 5. 登录按钮点击核心逻辑
    void slot_loginCheck();
    // 6. 忘记密码/注册账号 点击事件
    void slot_forgetPwd();
    bool slot_registerAccount();

private:
    // 初始化UI界面+控件属性
    void initUi();
    // 绑定所有信号槽（核心解耦点，所有关联写在这里）
    void initConnect();
    // 手机号合法性校验：11位纯数字
    bool checkPhoneLegal(const QString &phone);

private:
    // ========== 核心控件成员 ==========
    QLineEdit      *m_editPhone;      // 手机号输入框
    QLineEdit      *m_editPwd;        // 密码输入框
    QLineEdit      *m_editCode;       // 验证码输入框
    QPushButton    *m_btnGetCode;     // 获取验证码按钮
    QPushButton    *m_btnLogin;       // 登录按钮
    QPushButton    *m_btnForgetPwd;   // 忘记密码按钮
    QPushButton    *m_btnRegister;    // 注册账号按钮
    QRadioButton   *m_rBtnPwdLogin;   // 密码登录单选框
    QRadioButton   *m_rBtnCodeLogin;  // 验证码登录单选框
    QSlider        *m_sliderVerify;   // 滑块验证控件
    QLabel         *m_labSliderTip;   // 滑块提示文字

    // ========== 业务变量 ==========
    QTimer         *m_timerCountDown; // 验证码倒计时定时器
    int             m_nCountDown;     // 倒计时秒数（默认60）
    QString         m_strCurCode;     // 当前生成的6位验证码
    bool            m_bSliderOk;      // 滑块验证是否通过
    bool            m_bLoginSuccess;  // 登录是否成功
};

#endif // LOGINWIDGET_H
