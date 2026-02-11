#include "ForgetPwdDialog.h"
#include <QDebug>

ForgetPwdDialog::ForgetPwdDialog(const QString& phone, QWidget *parent)
    : QDialog(parent)
    , m_phone(phone)
{
    this->setWindowTitle("忘记密码 - 重置密码");
    this->setFixedSize(400, 350);
    m_smsHelper = new SmsHelper(this);
    // 绑定短信发送结果信号
    connect(m_smsHelper, &SmsHelper::sendResult, this, [&](bool success, const QString& msg) {
        if (success) {
            QMessageBox::information(this, "提示", msg);
        } else {
            QMessageBox::warning(this, "提示", msg);
            // 恢复“获取验证码”按钮状态
            m_getCodeBtn->setText("获取验证码");
            m_getCodeBtn->setEnabled(true);
            m_countDownTimer->stop();
            m_countDown = 60;
        }
    });
    initUi();
}

ForgetPwdDialog::~ForgetPwdDialog()
{
    if (m_countDownTimer->isActive()) {
        m_countDownTimer->stop();
    }
}

void ForgetPwdDialog::initUi()
{
    // 1. 初始化控件
    m_sliderVerify = new QSlider(Qt::Horizontal, this);
    m_sliderTip = new QLabel("→ 滑动验证后可获取验证码", this);
    m_codeEdit = new QLineEdit(this);
    m_getCodeBtn = new QPushButton("获取验证码", this);
    m_newPwdEdit = new QLineEdit(this);
    m_confirmPwdEdit = new QLineEdit(this);
    m_confirmBtn = new QPushButton("确认重置", this);
    m_cancelBtn = new QPushButton("取消", this);
    m_countDownTimer = new QTimer(this);
    m_countDownTimer->setInterval(1000);

    // 2. 控件属性设置
    // 滑块验证
    m_sliderVerify->setRange(0, 100);
    m_sliderVerify->setValue(0);
    m_sliderTip->setStyleSheet("color: #666; font-size: 12px;");

    // 验证码输入框
    m_codeEdit->setPlaceholderText("请输入6位验证码");
    m_codeEdit->setMaxLength(6);
    m_codeEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]+"), this));
    m_codeEdit->setEnabled(false); // 滑块验证通过前禁用

    // 获取验证码按钮
    m_getCodeBtn->setEnabled(false);
    m_getCodeBtn->setMinimumHeight(30);

    // 密码输入框
    m_newPwdEdit->setPlaceholderText("请设置新密码（至少6位）");
    m_newPwdEdit->setEchoMode(QLineEdit::Password);
    m_confirmPwdEdit->setPlaceholderText("请确认新密码");
    m_confirmPwdEdit->setEchoMode(QLineEdit::Password);

    // 按钮样式
    m_confirmBtn->setMinimumHeight(35);
    m_confirmBtn->setStyleSheet("background-color: #1E90FF; color: white; border: none; border-radius: 4px;");
    m_cancelBtn->setMinimumHeight(35);

    // 3. 布局管理
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(15);

    // 滑块验证布局
    QHBoxLayout *sliderLayout = new QHBoxLayout;
    sliderLayout->addWidget(m_sliderVerify);
    sliderLayout->addWidget(m_sliderTip);

    // 验证码布局
    QHBoxLayout *codeLayout = new QHBoxLayout;
    codeLayout->addWidget(m_codeEdit);
    codeLayout->addWidget(m_getCodeBtn);

    // 按钮布局
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addWidget(m_confirmBtn);
    btnLayout->addWidget(m_cancelBtn);

    // 添加到主布局
    mainLayout->addLayout(sliderLayout);
    mainLayout->addLayout(codeLayout);
    mainLayout->addWidget(new QLabel("设置新密码：", this));
    mainLayout->addWidget(m_newPwdEdit);
    mainLayout->addWidget(m_confirmPwdEdit);
    mainLayout->addLayout(btnLayout);

    this->setLayout(mainLayout);

    // 4. 绑定信号槽
    connect(m_sliderVerify, &QSlider::sliderReleased, this, &ForgetPwdDialog::slot_sliderVerify);
    connect(m_getCodeBtn, &QPushButton::clicked, this, &ForgetPwdDialog::slot_getCode);
    connect(m_countDownTimer, &QTimer::timeout, this, &ForgetPwdDialog::slot_countDown);
    connect(m_confirmBtn, &QPushButton::clicked, this, &ForgetPwdDialog::slot_confirmReset);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

// 滑块验证逻辑
void ForgetPwdDialog::slot_sliderVerify()
{
    if (m_sliderVerify->value() == 100) {
        m_sliderOk = true;
        m_sliderTip->setText("验证通过 ✔");
        m_sliderTip->setStyleSheet("color: #00CD00; font-size: 12px;");
        m_getCodeBtn->setEnabled(true); // 启用获取验证码按钮
        m_codeEdit->setEnabled(true);   // 启用验证码输入框
    } else {
        m_sliderOk = false;
        m_sliderTip->setText("→ 滑动验证后可获取验证码");
        m_sliderTip->setStyleSheet("color: #666; font-size: 12px;");
        m_getCodeBtn->setEnabled(false);
        m_codeEdit->setEnabled(false);
        m_sliderVerify->setValue(0); // 滑块归位
        QMessageBox::warning(this, "提示", "请滑动到最右侧完成验证！");
    }
}

// 获取验证码逻辑
void ForgetPwdDialog::slot_getCode()
{
    // 生成6位随机验证码
//    m_curCode = QString::number(QRandomGenerator::global()->bounded(100000, 999999));
//    qDebug() << "忘记密码验证码：" << m_curCode; // 实际项目替换为短信接口

    // 启动倒计时
    m_getCodeBtn->setEnabled(false);
    m_countDownTimer->start();
    m_getCodeBtn->setText(QString("重新获取(%1s)").arg(m_countDown));

    QMessageBox::information(this, "验证码已发送", QString("验证码已发送至 %1，请查收！").arg(m_phone));
    // 调用短信API发送验证码
    m_smsHelper->sendVerificationCode(m_phone, m_sendCode);
}

// 验证码倒计时
void ForgetPwdDialog::slot_countDown()
{
    m_countDown--;
    m_getCodeBtn->setText(QString("重新获取(%1s)").arg(m_countDown));
    if (m_countDown <= 0) {
        m_countDownTimer->stop();
        m_getCodeBtn->setText("获取验证码");
        m_getCodeBtn->setEnabled(true);
        m_countDown = 60;
    }
}

// 校验密码合法性（可选：自定义复杂度）
bool ForgetPwdDialog::checkPwdLegal(const QString& pwd)
{
    if (pwd.length() < 6) {
        QMessageBox::warning(this, "提示", "新密码长度至少6位！");
        return false;
    }
    // 可选：添加数字+字母组合校验
    // QRegExp regExp("^(?=.*[0-9])(?=.*[a-zA-Z]).{6,}$");
    // if (!regExp.exactMatch(pwd)) {
    //     QMessageBox::warning(this, "提示", "新密码需包含数字和字母！");
    //     return false;
    // }
    return true;
}

// 确认重置密码
void ForgetPwdDialog::slot_confirmReset()
{
    // 1. 校验验证码
    QString inputCode = m_codeEdit->text().trimmed();
    if (inputCode.isEmpty() || inputCode.length() != 6) {
        QMessageBox::warning(this, "提示", "请输入6位数字验证码！");
        return;
    }
//    if (inputCode != m_curCode) {
//        QMessageBox::warning(this, "提示", "验证码错误，请重新输入！");
//        m_codeEdit->clear();
//        return;
//    }
    // 改用真实发送的验证码校验
    if (inputCode != m_sendCode) {
        QMessageBox::warning(this, "提示", "验证码错误，请重新输入！");
        m_codeEdit->clear();
        return;
    }

    // 2. 校验新密码
    QString newPwd = m_newPwdEdit->text().trimmed();
    QString confirmPwd = m_confirmPwdEdit->text().trimmed();
    if (!checkPwdLegal(newPwd)) {
        return;
    }
    if (newPwd != confirmPwd) {
        QMessageBox::warning(this, "提示", "两次输入的新密码不一致！");
        m_confirmPwdEdit->clear();
        return;
    }

    // 3. 调用数据库修改密码
    UserDbHelper dbHelper;
    if (dbHelper.modifyUserPwdByPhone(m_phone, newPwd)) {
        QMessageBox::information(this, "成功", "密码重置成功！请使用新密码登录。");
        this->accept(); // 验证通过，关闭弹窗
    } else {
        QMessageBox::warning(this, "失败", "密码重置失败，请稍后重试！");
    }
}
