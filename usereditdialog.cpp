#include "usereditdialog.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include "userdbhelper.h"
#include "usersession.h"

UserEditDialog::UserEditDialog(QWidget *parent, DialogOperType type)
    : BaseEditDialog(parent, type)
{
    initUserForm();
    // 新建时：用户ID控件隐藏(自增主键，无需填写)；编辑时：用户ID只读
    m_editUserId->setVisible(type == UserEditDialog::Oper_Edit);
    m_editUserId->setReadOnly(true);
}

void UserEditDialog::initUserForm()
{
    // 初始化表单控件
    m_editUserId = new QLineEdit(this);
    m_editUserId->setVisible(false);
    m_editUserName = new QLineEdit(this);
    m_editNickName = new QLineEdit(this);
    m_cbxRoleName = new QComboBox(this);
    m_editPhone = new QLineEdit(this);
    m_editPwd = new QLineEdit(this);
    m_cbxStatus = new QComboBox(this);

    // 下拉框赋值，与数据库枚举值对应
    m_cbxRoleName->addItems({"超级管理员","普通用户"});
    m_cbxStatus->addItem("启用",0);
    m_cbxStatus->addItem("禁用",1);
    if(!UserSession::instance()->isUserLogin()) {
        m_cbxRoleName->setCurrentIndex(1);
        m_cbxRoleName->setEnabled(false);
        // 设置默认选中项
        m_cbxStatus->setCurrentIndex(0);
        // 禁用该下拉框（失能）
        m_cbxStatus->setVisible(false);
    }

    // ========== 创建表单布局 ==========
    QFormLayout *formLayout = new QFormLayout;
    // 设置表单标签宽度，对齐方式，优化显示效果
    formLayout->setLabelAlignment(Qt::AlignRight);
    formLayout->setHorizontalSpacing(10);
    formLayout->setVerticalSpacing(15);
    // 添加表单行：标签+控件，全部正常显示
    formLayout->addRow("用户ID",m_editUserId);
    formLayout->addRow("用户名",m_editUserName);
    formLayout->addRow("用户昵称",m_editNickName);
    formLayout->addRow("用户角色",m_cbxRoleName);
    formLayout->addRow("手机号",m_editPhone);
    formLayout->addRow("密码",m_editPwd);
    if(UserSession::instance()->isUserLogin()) {
        formLayout->addRow("账号状态",m_cbxStatus);
    }

    // 1. 父布局添加子布局 必须用 addLayout() 而不是 addItem()
    // 2. 先添加【表单布局】，再添加【按钮布局】，控件从上到下正常展示
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if(mainLayout)
    {
        mainLayout->insertLayout(0, formLayout); // 插入到布局第一个位置，最上方显示表单
    }
}

// 赋值：编辑用户时回显数据
void UserEditDialog::setFormData(int userId, QString userName, QString nickName, QString roleName, QString phone, QString pwd, int status)
{
    m_editUserId->setText(QString::number(userId));
    m_editUserName->setText(userName);
    m_editNickName->setText(nickName);
    m_cbxRoleName->setCurrentText(roleName);
    m_editPhone->setText(phone);
    m_editPwd->setText(pwd);
    m_cbxStatus->setCurrentIndex(status);
}

// 数据校验：必填项非空校验
bool UserEditDialog::checkFormData()
{
    if(m_editUserName->text().trimmed().isEmpty()){ QMessageBox::warning(this,"提示","登录账号不能为空！"); return false; }
    if(m_editNickName->text().trimmed().isEmpty()){ QMessageBox::warning(this,"提示","用户昵称不能为空！"); return false; }
    if(m_editPhone->text().trimmed().isEmpty()){ QMessageBox::warning(this,"提示","手机号不能为空！"); return false; }
    if(m_editPwd->text().trimmed().isEmpty()){ QMessageBox::warning(this,"提示","密码不能为空！"); return false; }
    return true;
}

// ========== 核心：数据库 新增+编辑 ==========
bool UserEditDialog::submitFormData()
{
    QString userName = m_editUserName->text().trimmed();
    QString nickName = m_editNickName->text().trimmed();
    QString roleName = m_cbxRoleName->currentText();
    QString phone = m_editPhone->text().trimmed();
    QString pwd = m_editPwd->text().trimmed();
    m_editPwd->echoMode();
    int status = m_cbxStatus->currentData().toInt();

    bool res = false;
    UserDbHelper userDbHelper;
    if(m_operType == UserEditDialog::Oper_Create)
    {
        // 调用数据库层接口-新增用户
        res = userDbHelper.addUser(userName, nickName, roleName, phone, pwd, status);
    }
    else if(m_operType == UserEditDialog::Oper_Edit)
    {
        // 调用数据库层接口-编辑用户
        int userId = m_editUserId->text().toInt();
        res = userDbHelper.updateUser(userId, userName, nickName, roleName, phone, pwd, status);
    }

    if(!res)
    {
        QMessageBox::critical(this,"操作失败", "数据库执行失败，请重试！");
    }
    return res;
}

// 数据赋值函数
void UserEditDialog::setUserId(const QString &id) { m_editUserId->setText(id); }
void UserEditDialog::setUserName(const QString &name) { m_editUserName->setText(name); }
void UserEditDialog::setUserRole(const QString &role) { m_cbxRoleName->setCurrentText(role); }
void UserEditDialog::setPhone(const QString &phone) { m_editPhone->setText(phone); }
void UserEditDialog::setState(const QString &state) { m_cbxStatus->setCurrentText(state); }
