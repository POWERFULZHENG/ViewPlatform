#include "BaseDbHelper.h"
#include <QSettings>
#include <QCoreApplication>
#include <QSqlError>
#include <QCryptographicHash>
#include <QDebug>
#include <QSqlQuery>
#include <QFile>

// 定义常量
const QString DB_CONNECT_NAME = "MYSQL_CONN";

// 初始化静态成员
BaseDbHelper* BaseDbHelper::m_pInstance = nullptr;
QMutex BaseDbHelper::m_mutex;

// 私有构造函数：仅初始化连接+读取配置
BaseDbHelper::BaseDbHelper(QObject *parent) : QObject(parent)
{
    // 初始化数据库连接
    if(QSqlDatabase::contains(DB_CONNECT_NAME))
    {
        m_db = QSqlDatabase::database(DB_CONNECT_NAME);
        // 复用连接时，也重新读取配置（防止配置修改后不生效）
        readDbConfig();
    }
    else
    {
        // 先创建连接 → 再读配置 → 配置参数直接绑定到已存在的m_db对象
        m_db = QSqlDatabase::addDatabase("QMYSQL", DB_CONNECT_NAME);
        readDbConfig(); // 读取配置文件
    }
    // 打印连接参数（调试用，确认配置生效）
    qDebug() << "✅ 数据库连接参数绑定成功："
             << "用户：" << m_db.userName()
             << "主机：" << m_db.hostName()
             << "库名：" << m_db.databaseName();
}

// 析构函数：安全关闭连接
BaseDbHelper::~BaseDbHelper()
{
    if(m_db.isOpen())
    {
        m_db.close();
    }
    releaseInstance();
}

// 读取数据库配置（通用层内部逻辑）
void BaseDbHelper::readDbConfig()
{
    QString configPath = QCoreApplication::applicationDirPath() + "/config.ini";
    if(!QFile::exists(configPath))
    {
        qCritical() << "配置文件不存在：" << configPath << "，使用默认参数！";
        return;
    }

    QSettings config(configPath, QSettings::IniFormat);
    config.beginGroup("Database");
    // 读取配置项，可带默认值兜底
    QString hostName = config.value("HostName").toString();
    int port = config.value("Port").toInt();
    QString dbName = config.value("DatabaseName").toString();
    QString userName = config.value("UserName").toString();
    QString password = config.value("Password").toString();
    config.endGroup();

    // 设置连接参数
    m_db.setHostName(hostName);
    m_db.setPort(port);
    m_db.setDatabaseName(dbName);
    m_db.setUserName(userName);
    m_db.setPassword(password);
    m_db.setConnectOptions("MYSQL_OPT_CONNECT_TIMEOUT=30"); // 连接超时
}

// 线程安全的单例获取
BaseDbHelper *BaseDbHelper::getInstance()
{
    if(m_pInstance == nullptr)
    {
        m_mutex.lock();
        if(m_pInstance == nullptr)
        {
            m_pInstance = new BaseDbHelper();
        }
        m_mutex.unlock();
    }
    return m_pInstance;
}

// 释放单例内存
void BaseDbHelper::releaseInstance()
{
    m_mutex.lock();
    if(m_pInstance != nullptr)
    {
        delete m_pInstance;
        m_pInstance = nullptr;
    }
    m_mutex.unlock();
}

// 连接检测+自动重连
bool BaseDbHelper::checkDbConn()
{
    if(m_db.isOpen()) return true;
    bool ok = m_db.open();
    if(!ok)
    {
        qCritical() << "数据库连接失败：" << getLastError();
    }
    return ok;
}

// MD5密码加密（通用工具）
QString BaseDbHelper::encryptPwd(const QString &pwd)
{
    // 1. 拼接盐值和原始密码（顺序固定，登录时需保持一致）
    QString salt = QString("QT_PASSWORD_SALT");
    QString content = salt + pwd;

    // 2. 选择加密算法（SHA256 推荐，替换为 QCryptographicHash::Md5 即为MD5加密）
    QByteArray encryptBytes = QCryptographicHash::hash(content.toUtf8(), QCryptographicHash::Sha256);

    // 3. 转换为十六进制字符串（方便存储和比对，无不可见字符）
    return encryptBytes.toHex();
}

// ========== 通用SQL执行接口（无业务逻辑） ==========
bool BaseDbHelper::execSql(const QString &sql)
{
    if(!checkDbConn()) return false;
    QSqlQuery query(m_db);
    return query.exec(sql);
}

QSqlQuery BaseDbHelper::execQuery(const QString &sql)
{
    QSqlQuery query(m_db);
    if(checkDbConn())
    {
        query.exec(sql);
    }
    return query;
}

bool BaseDbHelper::execPrepareSql(const QString &sql, const QVariantList &params)
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

QSqlQuery BaseDbHelper::execPrepareQuery(const QString &sql, const QVariantList &params)
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

// ========== 通用事务接口 ==========
bool BaseDbHelper::beginTransaction()
{
    QMutexLocker locker(&m_mutex);
    if(!checkDbConn()) return false;
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

bool BaseDbHelper::commitTransaction()
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
        m_db.rollback();
        return false;
    }
}

bool BaseDbHelper::rollbackTransaction()
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

bool BaseDbHelper::execBatchSql(const QStringList &sqlList)
{
    if(sqlList.isEmpty()) return false;
    if(!beginTransaction()) return false;

    QSqlQuery query(m_db);
    bool allSuccess = true;
    for(const QString &sql : sqlList)
    {
        if(!query.exec(sql))
        {
            allSuccess = false;
            qCritical() << "批量执行SQL失败：" << sql << "错误：" << query.lastError().text();
            break;
        }
    }

    return allSuccess ? commitTransaction() : rollbackTransaction();
}

bool BaseDbHelper::execBatchPrepareSql(const QString &sql, const QList<QVariantList> &paramsList)
{
    if(sql.isEmpty() || paramsList.isEmpty()) return false;
    if(!beginTransaction()) return false;

    QSqlQuery query(m_db);
    query.prepare(sql);
    bool allSuccess = true;
    for(const QVariantList &params : paramsList)
    {
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

    return allSuccess ? commitTransaction() : rollbackTransaction();
}

// 错误信息获取
QString BaseDbHelper::getLastError() const
{
    return QString("错误码：%1 \n错误信息：%2").arg(m_db.lastError().number()).arg(m_db.lastError().text());
}
