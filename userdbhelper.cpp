#include "UserDbHelper.h"
#include <QDebug>
#include <QSqlRecord>

UserDbHelper::UserDbHelper(QObject *parent) : QObject(parent)
{
    // 获取通用数据库层单例（复用连接）
    m_baseDbHelper = BaseDbHelper::getInstance();
}

// 用户注册：手机号+密码写入，校验重复
bool UserDbHelper::userRegister(const QString &phone, const QString &pwd)
{
    if(checkPhoneExist(phone)) return false;
    QString sql = "INSERT INTO sys_user(phone,pwd) VALUES (?,?)";
    return m_baseDbHelper->execPrepareSql(sql, {phone, m_baseDbHelper->encryptPwd(pwd)});
}

// 用户登录：校验手机号+密码
bool UserDbHelper::userLogin(const QString &phone, const QString &pwd)
{
//    qDebug() << m_baseDbHelper->encryptPwd(pwd);
    QString sql = "SELECT * FROM sys_user WHERE phone=? AND pwd=?";
    QSqlQuery query = m_baseDbHelper->execPrepareQuery(sql, {phone, m_baseDbHelper->encryptPwd(pwd)});
    return query.next();
}

// 校验手机号是否存在
bool UserDbHelper::checkPhoneExist(const QString &phone)
{
    QString sql = "SELECT * FROM sys_user WHERE phone=?";
    QSqlQuery query = m_baseDbHelper->execPrepareQuery(sql, {phone});
    return query.next();
}

// 修改密码：校验原密码后更新
bool UserDbHelper::modifyUserPwd(const QString &phone, const QString &oldPwd, const QString &newPwd)
{
    if(!checkPhoneExist(phone)) return false;
    // 校验原密码
    QString sqlCheck = "SELECT * FROM sys_user WHERE phone=? AND pwd=?";
    QSqlQuery query = m_baseDbHelper->execPrepareQuery(sqlCheck, {phone, m_baseDbHelper->encryptPwd(oldPwd)});
    if(!query.next()) return false;
    // 更新新密码
    QString sqlUpdate = "UPDATE sys_user SET pwd=? WHERE phone=?";
    return m_baseDbHelper->execPrepareSql(sqlUpdate, {m_baseDbHelper->encryptPwd(newPwd), phone});
}

// 根据手机号查询用户信息
QString UserDbHelper::getUserInfoByPhone(const QString &phone)
{
    QString sql = "SELECT phone FROM sys_user WHERE phone=?";
    QSqlQuery query = m_baseDbHelper->execPrepareQuery(sql, {phone});
    return query.next() ? query.value(0).toString() : "";
}

// 根据手机号查询用户信息
QString UserDbHelper::getUserUUIDByPhone(const QString &phone)
{
    QString sql = "SELECT id FROM sys_user WHERE phone=?";
    QSqlQuery query = m_baseDbHelper->execPrepareQuery(sql, {phone});
    return query.next() ? query.value(0).toString() : "";
}

// 返回值改为 QHash<QString, QString>，键=列名，值=列值
QHash<QString, QString> UserDbHelper::getUserInfoByUUID(const QString &UUID)
{
    // 定义返回的哈希对象（无匹配记录时，返回空QHash，无需额外处理）
    QHash<QString, QString> userInfoHash;

    // 1. SQL语句和查询参数
    QString sql = "SELECT * FROM sys_user WHERE phone=?";
    QSqlQuery query = m_baseDbHelper->execPrepareQuery(sql, {UUID});

    // 2. 判断是否查询到有效记录
    if (!query.next())
    {
        return userInfoHash; // 无匹配记录，返回空哈希
    }

    // 3. 获取查询结果的记录集（包含列名、列数信息）
    QSqlRecord record = query.record();
    int columnCount = record.count(); // 获取总列数

    // 4. 遍历所有列，将「列名-列值」存入哈希表
    for (int i = 0; i < columnCount; ++i)
    {
        // 键：列名（如"UUID"、"username"）
        QString columnName = record.fieldName(i);
        // 值：列对应的字段值（转为QString，兼容int、varchar等大部分类型）
        QString columnValue = query.value(i).toString();

        // 存入哈希表
        userInfoHash.insert(columnName, columnValue);
    }

    // 5. 返回填充完成的哈希表
    return userInfoHash;
}

// 删除用户
bool UserDbHelper::delUserById(int userId)
{
    QString sql = "DELETE FROM sys_user WHERE id=?";
    return m_baseDbHelper->execPrepareSql(sql, {userId});
}

// 添加用户
bool UserDbHelper::addUser(const QString &userName, const QString &nickName, const QString &roleName, const QString &phone, const QString &pwd, int status)
{
    QString sql = "INSERT INTO sys_user(user_name,nick_name,role_name,phone,pwd,status,create_time) VALUES (?,?,?,?,?,?,NOW())";
    return m_baseDbHelper->execPrepareSql(sql, {userName, nickName, roleName, phone, m_baseDbHelper->encryptPwd(pwd), status});
}

// 更新用户
bool UserDbHelper::updateUser(int userId, const QString &userName, const QString &nickName, const QString &roleName, const QString &phone, const QString &pwd, int status)
{
    QString sql = "UPDATE sys_user SET user_name=?,nick_name=?,role_name=?,phone=?,pwd=?,status=? WHERE id=?";
    return m_baseDbHelper->execPrepareSql(sql, {userName, nickName, roleName, phone, m_baseDbHelper->encryptPwd(pwd), status, userId});
}

// 获取所有用户列表
QSqlQuery UserDbHelper::getAllUserList()
{
    return m_baseDbHelper->execQuery("SELECT id,user_name,nick_name,role_name,phone,status,pwd,create_time FROM sys_user ORDER BY id DESC");
}

// 搜索用户
QSqlQuery UserDbHelper::searchUserByKey(const QString &key)
{
    QString sql = "SELECT * FROM sys_user WHERE user_name LIKE ? OR nick_name LIKE ? OR phone LIKE ? OR role_name LIKE ? ORDER BY id DESC";
    QString likeKey = "%" + key + "%";
    return m_baseDbHelper->execPrepareQuery(sql, {likeKey, likeKey, likeKey, likeKey});
}
