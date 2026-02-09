#ifndef USEREDITDIALOG_H
#define USEREDITDIALOG_H

#include "baseeditdialog.h"
#include <QLineEdit>
#include <QComboBox>
#include <QFormLayout>

class UserEditDialog : public BaseEditDialog
{
    Q_OBJECT
public:
    explicit UserEditDialog(QWidget *parent = nullptr, DialogOperType type = Oper_Create);
    ~UserEditDialog() override = default;

    // 给编辑页面赋值：回显选中行的用户数据
    void setFormData(int userId, QString userName, QString nickName, QString roleName, QString phone, QString pwd, int status);
    void setUserId(const QString &id);
    void setUserName(const QString &name);
    void setUserRole(const QString &role);
    void setPhone(const QString &phone);
    void setState(const QString &state);
protected:
    bool checkFormData() override;  // 实现父类纯虚函数：用户数据校验
    bool submitFormData() override; // 实现父类纯虚函数：数据库新增/编辑

private:
    // 用户表表单控件，与数据库字段一一对应
    QLineEdit *m_editUserId;     // 用户ID(主键，编辑只读)
    QLineEdit *m_editUserName;   // 用户名
    QLineEdit *m_editNickName;   // 用户昵称
    QComboBox *m_cbxRoleName;    // 角色名称
    QLineEdit *m_editPhone;      // 手机号
    QLineEdit *m_editPwd;      // 密码
    QComboBox *m_cbxStatus;      // 状态：0=禁用 1=启用

    // 初始化用户表单UI
    void initUserForm();
};

#endif // USEREDITDIALOG_H
