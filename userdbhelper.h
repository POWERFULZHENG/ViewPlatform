#ifndef USERDBHELPER_H
#define USERDBHELPER_H

#include <QObject>
#include <QSqlQuery>
#include <QHash>
#include "BaseDbHelper.h"

// 用户业务助手：仅处理用户相关业务逻辑，依赖通用数据库层
class UserDbHelper : public QObject
{
    Q_OBJECT
public:
    explicit UserDbHelper(QObject *parent = nullptr);

    // 基础用户操作（登录/注册/改密码）
    bool userRegister(const QString &phone, const QString &pwd);
    bool userLogin(const QString &phone, const QString &pwd);
    bool checkPhoneExist(const QString &phone);
    bool modifyUserPwd(const QString &phone, const QString &oldPwd, const QString &newPwd);
    QString getUserInfoByPhone(const QString &phone);
    QHash<QString, QString> getUserInfoByUUID(const QString &UUID);

    // 用户管理操作（增删改查）
    bool delUserById(int userId);
    bool addUser(const QString &userName, const QString &nickName, const QString &roleName, const QString &phone, const QString &pwd, int status);
    bool updateUser(int userId, const QString &userName, const QString &nickName, const QString &roleName, const QString &phone, const QString &pwd, int status);
    QSqlQuery getAllUserList();
    QSqlQuery searchUserByKey(const QString &key);

private:
    BaseDbHelper *m_baseDbHelper; // 依赖通用数据库层
};

#endif // USERDBHELPER_H
