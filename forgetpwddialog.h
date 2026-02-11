#ifndef FORGETPWDDIALOG_H
#define FORGETPWDDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QTimer>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QRegExpValidator>
#include <QRegExp>
#include "userdbhelper.h"
#include "smshelper.h"

class ForgetPwdDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ForgetPwdDialog(const QString& phone, QWidget *parent = nullptr);
    ~ForgetPwdDialog();

    // 获取最终输入的新密码（验证通过后调用）
    QString getNewPassword() const { return m_newPwdEdit->text().trimmed(); }

private slots:
    // 滑块验证
    void slot_sliderVerify();
    // 获取验证码
    void slot_getCode();
    // 验证码倒计时
    void slot_countDown();
    // 确认重置密码
    void slot_confirmReset();

private:
    // 初始化UI
    void initUi();
    // 校验密码合法性（可选：添加复杂度校验）
    bool checkPwdLegal(const QString& pwd);

private:
    QString m_phone;               // 要重置密码的手机号
    QString m_curCode;             // 当前生成的验证码
    int m_countDown = 60;          // 倒计时秒数
    bool m_sliderOk = false;       // 滑块验证状态
    SmsHelper *m_smsHelper;
    QString m_sendCode; // 存储发送的验证码（用于校验）

    // 控件
    QSlider *m_sliderVerify;       // 滑块验证
    QLabel *m_sliderTip;           // 滑块提示
    QLineEdit *m_codeEdit;         // 验证码输入框
    QPushButton *m_getCodeBtn;     // 获取验证码按钮
    QLineEdit *m_newPwdEdit;       // 新密码输入框
    QLineEdit *m_confirmPwdEdit;   // 确认密码输入框
    QPushButton *m_confirmBtn;     // 确认按钮
    QPushButton *m_cancelBtn;      // 取消按钮
    QTimer *m_countDownTimer;      // 倒计时定时器
};

#endif // FORGETPWDDIALOG_H
