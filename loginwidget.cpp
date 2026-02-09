#include "loginwidget.h"
#include "baseeditdialog.h"
#include "usereditdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QRegExp>
#include <QDebug>
#include <QInputDialog>
#include <QIcon>

LoginWidget::LoginWidget(QWidget *parent)
    : QDialog(parent)
    , m_nCountDown(60)
    , m_bSliderOk(false)
    , m_bLoginSuccess(false)
{
    this->setWindowTitle("用户登录");
    this->setFixedSize(420, 500); // 固定登录窗口大小，也可自适应
    this->setWindowIcon(QIcon(":/menu/static/icon/Docker.svg"));
    initUi();     // 初始化界面
    initConnect();// 绑定信号槽
}

// ===================== 私有核心方法：初始化界面 =====================
void LoginWidget::initUi()
{
    // 1. 初始化基础控件
    m_rBtnPwdLogin = new QRadioButton("手机号+密码登录");
    m_rBtnCodeLogin = new QRadioButton("手机号+验证码登录");
    m_editPhone = new QLineEdit;
    m_editPwd = new QLineEdit;
    m_editCode = new QLineEdit;
    m_btnGetCode = new QPushButton("获取验证码");
    m_btnLogin = new QPushButton("登 录");
    m_btnForgetPwd = new QPushButton("忘记密码？");
    m_btnRegister = new QPushButton("注册新账号");
    m_sliderVerify = new QSlider(Qt::Horizontal);
    m_labSliderTip = new QLabel("→ 滑动验证后可获取验证码");

    // 2. 控件属性设置
    m_editPhone->setPlaceholderText("请输入11位手机号");
    m_editPhone->setText("13432933342");
    m_editPhone->setMaxLength(11); // 手机号限制11位
    // 手机号仅允许输入数字
    m_editPhone->setValidator(new QRegExpValidator(QRegExp("[0-9]+"), this));
    m_editPwd->setPlaceholderText("请输入登录密码");
    m_editPwd->setEchoMode(QLineEdit::Password); // 密码密文显示
    m_editPwd->setText("123456");
    m_editCode->setPlaceholderText("请输入6位验证码");
    m_editCode->setMaxLength(6);
    m_editCode->setValidator(new QRegExpValidator(QRegExp("[0-9]+"), this));

    m_btnGetCode->setEnabled(false); // 默认禁用，滑块验证通过后启用
    m_btnGetCode->setMinimumHeight(30);
    m_btnLogin->setMinimumHeight(35);
    m_btnLogin->setStyleSheet("background-color: #1E90FF; color: white; border: none; border-radius: 4px;");

    m_sliderVerify->setRange(0, 100); // 滑块范围0-100
    m_sliderVerify->setValue(0);      // 默认滑块归位
    m_labSliderTip->setStyleSheet("color: #666666; font-size: 12px;");

    m_rBtnPwdLogin->setChecked(true); // 默认选中【密码登录】
    m_editCode->hide();               // 默认隐藏验证码输入框
    m_btnGetCode->hide();             // 默认隐藏获取验证码按钮
    m_sliderVerify->hide();           // 默认隐藏滑块验证
    m_labSliderTip->hide();

    // 3. 布局管理：核心布局（外层垂直布局，内层水平布局）
    QVBoxLayout *vMainLayout = new QVBoxLayout(this);
    vMainLayout->setContentsMargins(40, 50, 40, 50);
    vMainLayout->setSpacing(20);

    // 登录方式单选框布局
    QHBoxLayout *hRadioLayout = new QHBoxLayout;
    hRadioLayout->addWidget(m_rBtnPwdLogin);
    hRadioLayout->addWidget(m_rBtnCodeLogin);

    // 滑块验证布局
    QHBoxLayout *hSliderLayout = new QHBoxLayout;
    hSliderLayout->addWidget(m_sliderVerify);
    hSliderLayout->addWidget(m_labSliderTip);

    // 忘记密码+注册账号布局
    QHBoxLayout *hBtnLayout = new QHBoxLayout;
    hBtnLayout->addWidget(m_btnForgetPwd);
    hBtnLayout->addStretch();
    hBtnLayout->addWidget(m_btnRegister);

    // 添加所有控件到主布局
    vMainLayout->addLayout(hRadioLayout);
    vMainLayout->addWidget(m_editPhone);
    vMainLayout->addWidget(m_editPwd);
    vMainLayout->addWidget(m_editCode);
    vMainLayout->addWidget(m_btnGetCode);
    vMainLayout->addLayout(hSliderLayout);
    vMainLayout->addWidget(m_btnLogin);
    vMainLayout->addStretch();
    vMainLayout->addLayout(hBtnLayout);

    // 初始化倒计时定时器
    m_timerCountDown = new QTimer(this);
    m_timerCountDown->setInterval(1000); // 定时1秒
}

// ===================== 私有核心方法：信号槽绑定 =====================
void LoginWidget::initConnect()
{
    // 登录方式切换
    connect(m_rBtnPwdLogin, &QRadioButton::clicked, this, &LoginWidget::slot_switchLoginMode);
    connect(m_rBtnCodeLogin, &QRadioButton::clicked, this, &LoginWidget::slot_switchLoginMode);
    // 滑块验证
    connect(m_sliderVerify, &QSlider::sliderReleased, this, &LoginWidget::slot_sliderVerify);
    // 获取验证码+倒计时
    connect(m_btnGetCode, &QPushButton::clicked, this, &LoginWidget::slot_getVerificationCode);
    connect(m_timerCountDown, &QTimer::timeout, this, &LoginWidget::slot_countDownTime);
    // 登录按钮
    connect(m_btnLogin, &QPushButton::clicked, this, &LoginWidget::slot_loginCheck);
    // 忘记密码/注册
    connect(m_btnForgetPwd, &QPushButton::clicked, this, &LoginWidget::slot_forgetPwd);
    connect(m_btnRegister, &QPushButton::clicked, this, &LoginWidget::slot_registerAccount);
}

// ===================== 私有工具方法：手机号合法性校验 =====================
bool LoginWidget::checkPhoneLegal(const QString &phone)
{
    return phone.length() == 11 && phone.toLongLong() > 0;
}

// ===================== 槽函数：登录方式切换 =====================
void LoginWidget::slot_switchLoginMode()
{
    if(m_rBtnPwdLogin->isChecked())
    {
        // 切换到【密码登录】：显示密码框，隐藏验证码相关控件
        m_editPwd->show();
        m_editCode->hide();
        m_btnGetCode->hide();
        m_sliderVerify->hide();
        m_labSliderTip->hide();
        m_sliderVerify->setValue(0); // 滑块归位
        m_bSliderOk = false;
    }
    else
    {
        // 切换到【验证码登录】：隐藏密码框，显示验证码相关控件
        m_editPwd->hide();
        m_editCode->show();
        m_btnGetCode->show();
        m_sliderVerify->show();
        m_labSliderTip->show();
    }
    // 切换后清空输入框
    m_editCode->clear();
    m_editPwd->clear();
}

// ===================== 槽函数：滑块验证核心逻辑 =====================
void LoginWidget::slot_sliderVerify()
{
    // 滑块滑到最右侧（值=100），验证通过
    if(m_sliderVerify->value() == 100)
    {
        m_bSliderOk = true;
        m_btnGetCode->setEnabled(true);
        m_labSliderTip->setText("验证通过 ✔");
        m_labSliderTip->setStyleSheet("color: #00CD00; font-size: 12px;");
    }
    else
    {
        m_bSliderOk = false;
        m_btnGetCode->setEnabled(false);
        m_labSliderTip->setText("→ 滑动验证后可获取验证码");
        m_labSliderTip->setStyleSheet("color: #666666; font-size: 12px;");
        m_sliderVerify->setValue(0); // 滑块归位
    }
}

// ===================== 槽函数：获取验证码+倒计时启动 =====================
void LoginWidget::slot_getVerificationCode()
{
    QString strPhone = m_editPhone->text().trimmed();
    if(!checkPhoneLegal(strPhone))
    {
        QMessageBox::warning(this, "提示", "请输入合法的11位手机号！");
        return;
    }

    if(!m_bSliderOk)
    {
        QMessageBox::warning(this, "提示", "请先完成滑块验证！");
        return;
    }

    // 生成6位随机验证码（QT5.14+推荐QRandomGenerator，替代旧的qrand）
    m_strCurCode = QString::number(QRandomGenerator::global()->bounded(100000, 999999));
    qDebug() << "当前验证码：" << m_strCurCode; // 调试用，实际项目注释掉

    // 启动倒计时+禁用按钮
    m_btnGetCode->setEnabled(false);
    m_nCountDown = 60;
    m_timerCountDown->start();
    m_btnGetCode->setText(QString("重新获取(%1s)").arg(m_nCountDown));

    QMessageBox::information(this, "验证码已发送", QString("验证码已发送至 %1，请注意查收！").arg(strPhone));
}

// ===================== 槽函数：验证码倒计时 =====================
void LoginWidget::slot_countDownTime()
{
    m_nCountDown--;
    m_btnGetCode->setText(QString("重新获取(%1s)").arg(m_nCountDown));
    if(m_nCountDown <= 0)
    {
        m_timerCountDown->stop();
        m_btnGetCode->setText("获取验证码");
        m_btnGetCode->setEnabled(true);
        m_nCountDown = 60;
    }
}

// ===================== 槽函数：核心登录校验逻辑 =====================
void LoginWidget::slot_loginCheck()
{
    QString strPhone = m_editPhone->text().trimmed();
    if(!checkPhoneLegal(strPhone))
    {
        QMessageBox::warning(this, "登录失败", "请输入合法的11位手机号！");
        return;
    }

    // 数据库查询
    UserDbHelper userDbHelper;

    // 分支1：密码登录
    if(m_rBtnPwdLogin->isChecked())
    {
        QString strPwd = m_editPwd->text().trimmed();
        if(strPwd.isEmpty())
        {
            QMessageBox::warning(this, "登录失败", "请输入登录密码！");
            return;
        }
        // 示例：手机号13800138000 + 密码123456 视为登录成功
        if(userDbHelper.userLogin(strPhone, strPwd))
        {
            m_bLoginSuccess = true;
            m_curLoginPhone = strPhone; // ====== 赋值当前登录手机号 ======
            QMessageBox::information(this, "登录成功", "密码登录成功，即将进入主界面！");
            this->accept(); // 新增这一行，设置对话框的返回值为 Accepted
            this->close(); // 关闭登录窗口
        }
        else
        {
            QMessageBox::warning(this, "登录失败", "手机号或密码错误，请重试！");
            m_editPwd->clear();
        }
    }
    // 分支2：验证码登录
    else
    {
        QString strCode = m_editCode->text().trimmed();
        if(strCode.isEmpty() || strCode.length() !=6)
        {
            QMessageBox::warning(this, "登录失败", "请输入6位数字验证码！");
            return;
        }
        // ========== 此处替换为你的真实验证码校验逻辑 ==========
        if(strCode == m_strCurCode && userDbHelper.checkPhoneExist(strPhone))
        {
            m_bLoginSuccess = true;
            m_curLoginPhone = strPhone; // ====== 新增：赋值当前登录手机号 ======
            QMessageBox::information(this, "登录成功", "验证码登录成功，即将进入主界面！");
            this->accept(); // 新增这一行，设置对话框的返回值为 Accepted
            this->close(); // 关闭登录窗口
        }
        else if(!userDbHelper.checkPhoneExist(strPhone))
        {
            QMessageBox::warning(this, "登录失败", "该手机号未注册，请先注册！");
            m_editCode->clear();
        }
        else
        {
            QMessageBox::warning(this, "登录失败", "验证码错误，请重试！");
            m_editCode->clear();
        }
    }
}


// ========== 核心修改：忘记密码 对接MySQL数据库 ==========
void LoginWidget::slot_forgetPwd()
{
    QString strPhone = m_editPhone->text().trimmed();
    if(!checkPhoneLegal(strPhone))
    {
        QMessageBox::warning(this, "提示", "请先输入合法的11位手机号！");
        return;
    }
    // 获取原密码和新密码
    QString strOldPwd = QInputDialog::getText(this, "忘记密码", "请输入原密码：", QLineEdit::Password);
    if(strOldPwd.isEmpty()) return;
    QString strNewPwd = QInputDialog::getText(this, "忘记密码", "请输入新密码：", QLineEdit::Password);
    if(strNewPwd.isEmpty()) return;
    QString strConfirmPwd = QInputDialog::getText(this, "忘记密码", "请确认新密码：", QLineEdit::Password);
    if(strConfirmPwd != strNewPwd)
    {
        QMessageBox::warning(this, "提示", "两次输入的新密码不一致！");
        return;
    }
    // 调用数据库修改密码
    UserDbHelper userDbHelper;
    userDbHelper.modifyUserPwd(strPhone, strOldPwd, strNewPwd);
    m_editPwd->clear();
}

// ========== 核心修改：注册账号 对接MySQL数据库 ==========
bool LoginWidget::slot_registerAccount()
{
    UserEditDialog userRegisterDialog(this, BaseEditDialog::Oper_Create);
    userRegisterDialog.setWindowTitle("注册新用户");
    qDebug() << "slot_registerAccount" ;
    return userRegisterDialog.exec() == QDialog::Accepted;
//    QString strPhone = m_editPhone->text().trimmed();
//    if(!checkPhoneLegal(strPhone))
//    {
//        QMessageBox::warning(this, "提示", "请先输入合法的11位手机号！");
//        return;
//    }
//    // 获取注册密码
//    QString strPwd = QInputDialog::getText(this, "用户注册", "请设置登录密码：", QLineEdit::Password);
//    if(strPwd.isEmpty()) return;
//    QString strConfirmPwd = QInputDialog::getText(this, "用户注册", "请确认登录密码：", QLineEdit::Password);
//    if(strConfirmPwd != strPwd)
//    {
//        QMessageBox::warning(this, "提示", "两次输入的密码不一致！");
//        return;
//    }
//    // 调用数据库注册接口
//    UserDbHelper userDbHelper;
//    userDbHelper.userRegister(strPhone, strPwd);
//    m_editPwd->clear();
}
