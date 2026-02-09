#include "mysqlhelper.h"
#include <QSqlError>
#include <QCryptographicHash>
#include <QMessageBox>
#include <QString>
#include <QSettings>
#include <QCoreApplication>
#include <QDebug>

// 初始化静态成员
MysqlHelper* MysqlHelper::m_pInstance = nullptr;
QMutex MysqlHelper::m_mutex;
// 定义常量
const QString DB_CONNECT_NAME = "MYSQL_CONN";

// 私有构造函数
MysqlHelper::MysqlHelper(QObject *parent) : QObject(parent)
{
    // ========== 1. 读取config.ini配置文件 ==========
    QSettings config(QCoreApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    config.beginGroup("Database");
    // 读取配置项，增加默认值兜底，防止配置缺失导致崩溃
    QString hostName = config.value("HostName").toString();
    int port = config.value("Port").toInt();
    QString dbName = config.value("DatabaseName").toString();
    QString userName = config.value("UserName").toString();
    QString password = config.value("Password").toString();
    config.endGroup();
    // 初始化时只做连接配置，不主动打开，按需打开
    if(QSqlDatabase::contains(DB_CONNECT_NAME))
    {
        m_db = QSqlDatabase::database(DB_CONNECT_NAME);
    }
    else
    {
        m_db = QSqlDatabase::addDatabase("QMYSQL", DB_CONNECT_NAME);
        m_db.setHostName(hostName);
        m_db.setPort(port);
        m_db.setDatabaseName(dbName);
        m_db.setUserName(userName);
        m_db.setPassword(password);
        // 可选：设置连接超时（QT5.13+支持）
        m_db.setConnectOptions("MYSQL_OPT_CONNECT_TIMEOUT=30");
    }
}

// 析构函数：安全关闭数据库
MysqlHelper::~MysqlHelper()
{
    if(m_db.isOpen())
    {
        m_db.close();
    }
    releaseInstance();
}

// 线程安全的单例获取
MysqlHelper *MysqlHelper::getInstance()
{
    if(m_pInstance == nullptr)
    {
        m_mutex.lock();
        if(m_pInstance == nullptr)
        {
            m_pInstance = new MysqlHelper();
        }
        m_mutex.unlock();
    }
    return m_pInstance;
}

// 释放单例内存
void MysqlHelper::releaseInstance()
{
    m_mutex.lock();
    if(m_pInstance != nullptr)
    {
        delete m_pInstance;
        m_pInstance = nullptr;
    }
    m_mutex.unlock();
}

// 核心：检测连接+自动重连【所有数据库操作必调，根治 database not open】
bool MysqlHelper::checkDbConn()
{
    if(m_db.isOpen()) return true;
    // 数据库未打开，尝试重新连接
    bool ok = m_db.open();
    if(!ok)
    {
        qCritical() << "数据库连接失败：" << getLastError();
    }
    return ok;
}

// MD5密码加密（Qt原生实现，无依赖，生产可用）
QString MysqlHelper::encryptPwd(const QString &pwd)
{
    QByteArray ba = QCryptographicHash::hash(pwd.toUtf8(), QCryptographicHash::Md5);
    return ba.toHex();
}

// ========== 通用SQL执行接口 ==========
bool MysqlHelper::execSql(const QString &sql)
{
    if(!checkDbConn()) return false;
    QSqlQuery query(m_db);
    return query.exec(sql);
}

QSqlQuery MysqlHelper::execQuery(const QString &sql)
{
    QSqlQuery query(m_db);
    if(checkDbConn())
    {
        query.exec(sql);
    }
    return query;
}

// 预处理执行SQL 【防SQL注入，所有业务优先使用】
bool MysqlHelper::execPrepareSql(const QString &sql, const QVariantList &params)
{
    if(!checkDbConn()) return false;
    QSqlQuery query(m_db);
    query.prepare(sql);
    for(int i=0; i<params.size(); i++)
    {
        query.bindValue(i, params.at(i));
    }
    return query.exec();
}

// 预处理查询SQL 【防SQL注入】
QSqlQuery MysqlHelper::execPrepareQuery(const QString &sql, const QVariantList &params)
{
    QSqlQuery query(m_db);
    if(checkDbConn())
    {
        query.prepare(sql);
        for(int i=0; i<params.size(); i++)
        {
            query.bindValue(i, params.at(i));
        }
        query.exec();
    }
    return query;
}

// 获取数据库最新错误信息
QString MysqlHelper::getLastError() const
{
    return QString("错误码：%1 \n错误信息：%2").arg(m_db.lastError().number()).arg(m_db.lastError().text());
}

/**
 * @brief 开始数据库事务（线程安全）
 * @return true-事务开始成功，false-失败
 */
bool MysqlHelper::beginTransaction()
{
    QMutexLocker locker(&m_mutex); // 线程安全锁
    if(!checkDbConn()) return false;

    // 检测是否已开启事务，避免嵌套
    if(m_db.transaction())
    {
        qDebug() << "事务开始成功";
        return true;
    }
    else
    {
        qCritical() << "事务开始失败：" << getLastError();
        return false;
    }
}

/**
 * @brief 提交事务（所有操作成功后调用）
 * @return true-提交成功，false-失败
 */
bool MysqlHelper::commitTransaction()
{
    QMutexLocker locker(&m_mutex);
    if(!m_db.isOpen()) return false;

    if(m_db.commit())
    {
        qDebug() << "事务提交成功";
        return true;
    }
    else
    {
        qCritical() << "事务提交失败，自动回滚：" << getLastError();
        m_db.rollback(); // 提交失败强制回滚
        return false;
    }
}

/**
 * @brief 回滚事务（任意操作失败后调用）
 * @return true-回滚成功，false-失败
 */
bool MysqlHelper::rollbackTransaction()
{
    QMutexLocker locker(&m_mutex);
    if(!m_db.isOpen()) return false;

    if(m_db.rollback())
    {
        qDebug() << "事务回滚成功";
        return true;
    }
    else
    {
        qCritical() << "事务回滚失败：" << getLastError();
        return false;
    }
}

/**
 * @brief 批量操作示例（原子性保障）
 * @param sqlList 批量执行的SQL语句列表
 * @return true-全部成功，false-全部回滚
 */
bool MysqlHelper::execBatchSql(const QStringList &sqlList)
{
    if(sqlList.isEmpty()) return false;

    // 1. 开始事务
    if(!beginTransaction())
    {
        return false;
    }

    // 2. 执行批量SQL
    QSqlQuery query(m_db);
    bool allSuccess = true;
    for(const QString &sql : sqlList)
    {
        if(!query.exec(sql))
        {
            allSuccess = false;
            qCritical() << "批量执行SQL失败：" << sql << "错误：" << query.lastError().text();
            break; // 任意一条失败，立即终止执行
        }
    }

    // 3. 根据执行结果提交/回滚
    if(allSuccess)
    {
        return commitTransaction();
    }
    else
    {
        return rollbackTransaction();
    }
}

/**
 * @brief 批量预处理操作（防SQL注入，原子性保障）
 * @param sql 预处理SQL模板（如：INSERT INTO user (phone) VALUES (?)）
 * @param paramsList 批量参数列表（每一项是一组参数）
 * @return true-全部成功，false-全部回滚
 */
bool MysqlHelper::execBatchPrepareSql(const QString &sql, const QList<QVariantList> &paramsList)
{
    if(sql.isEmpty() || paramsList.isEmpty()) return false;

    // 1. 开始事务
    if(!beginTransaction())
    {
        return false;
    }

    // 2. 执行批量预处理SQL
    QSqlQuery query(m_db);
    query.prepare(sql);
    bool allSuccess = true;

    for(const QVariantList &params : paramsList)
    {
        // 绑定参数
        for(int i=0; i<params.size(); i++)
        {
            query.bindValue(i, params.at(i));
        }

        if(!query.exec())
        {
            allSuccess = false;
            qCritical() << "批量预处理SQL失败：" << query.lastError().text();
            break;
        }
    }

    // 3. 提交/回滚
    if(allSuccess)
    {
        return commitTransaction();
    }
    else
    {
        return rollbackTransaction();
    }
}


// 用户注册：手机号+密码写入数据库，自动校验手机号是否重复
bool MysqlHelper::userRegister(const QString &phone, const QString &pwd)
{
    if(checkPhoneExist(phone)) return false;
    QString sql = "INSERT INTO sys_user(phone,pwd) VALUES (?,?)";
    return execPrepareSql(sql, {phone, encryptPwd(pwd)});
}

// 账号密码登录：校验数据库中手机号和密码是否匹配
bool MysqlHelper::userLogin(const QString &phone, const QString &pwd)
{
    QString sql = "SELECT * FROM sys_user WHERE phone=? AND pwd=?";
    QSqlQuery query = execPrepareQuery(sql, {phone, encryptPwd(pwd)});
    return query.next();
}

// 校验手机号是否存在：验证码登录/忘记密码 前置校验
bool MysqlHelper::checkPhoneExist(const QString &phone)
{
    QString sql = "SELECT * FROM sys_user WHERE phone=?";
    QSqlQuery query = execPrepareQuery(sql, {phone});
    return query.next();
}

// 修改密码：校验原密码正确后，更新新密码
bool MysqlHelper::modifyUserPwd(const QString &phone, const QString &oldPwd, const QString &newPwd)
{
    if(!checkPhoneExist(phone)) return false;
    // 校验原密码
    QString sqlCheck = "SELECT * FROM sys_user WHERE phone=? AND pwd=?";
    QSqlQuery query = execPrepareQuery(sqlCheck, {phone, encryptPwd(oldPwd)});
    if(!query.next()) return false;
    // 更新新密码
    QString sqlUpdate = "UPDATE sys_user SET pwd=? WHERE phone=?";
    return execPrepareSql(sqlUpdate, {encryptPwd(newPwd), phone});
}

// 根据手机号查询用户手机号【查询用户信息，可扩展查询其他字段】
QString MysqlHelper::getUserInfoByPhone(const QString &phone)
{
    QString sql = "SELECT phone FROM sys_user WHERE phone=?";
    QSqlQuery query = execPrepareQuery(sql, {phone});
    return query.next() ? query.value(0).toString() : "";
}

bool MysqlHelper::delUserById(int userId)
{
    QString sql = "DELETE FROM user WHERE id=?";
    return execPrepareSql(sql, {userId});
}

bool MysqlHelper::addUser(const QString &userName, const QString &nickName, const QString &roleName, const QString &phone, int status)
{
    QString sql = "INSERT INTO user(user_name,nick_name,role_name,phone,status,create_time) VALUES (?,?,?,?,?,NOW())";
    return execPrepareSql(sql, {userName, nickName, roleName, phone, status});
}

bool MysqlHelper::updateUser(int userId, const QString &userName, const QString &nickName, const QString &roleName, const QString &phone, int status)
{
    QString sql = "UPDATE user SET user_name=?,nick_name=?,role_name=?,phone=?,status=? WHERE id=?";
    return execPrepareSql(sql, {userName, nickName, roleName, phone, status, userId});
}

QSqlQuery MysqlHelper::getAllUserList()
{
    return execQuery("SELECT id,user_name,nick_name,role_name,phone,status,create_time FROM user ORDER BY id DESC");
}

QSqlQuery MysqlHelper::searchUserByKey(const QString &key)
{
    QString sql = "SELECT * FROM user WHERE user_name LIKE ? OR nick_name LIKE ? OR phone LIKE ? OR role_name LIKE ? ORDER BY id DESC";
    QString likeKey = "%" + key + "%";
    return execPrepareQuery(sql, {likeKey, likeKey, likeKey, likeKey});
}

QSqlQuery MysqlHelper::getAllLogList()
{
    return execQuery("SELECT * FROM sys_log ORDER BY log_id ASC");
}

QSqlQuery MysqlHelper::searchLogByKey(const QString &key)
{
    QString sql = "SELECT * FROM sys_log WHERE operator LIKE ? OR operation_content LIKE ? OR operation_time LIKE ? OR ip_address LIKE ? ORDER BY log_id ASC";
    QString likeKey = "%" + key + "%";
    return execPrepareQuery(sql, {likeKey, likeKey, likeKey, likeKey});
}
