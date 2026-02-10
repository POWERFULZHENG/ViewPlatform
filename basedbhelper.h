#ifndef BASEDBHELPER_H
#define BASEDBHELPER_H

#include <QObject>
#include <QSqlDatabase>
#include <QMutex>
#include <QStringList>
#include <QVariantList>

// 通用数据库助手：仅处理连接、事务、通用SQL执行，无任何业务逻辑
class BaseDbHelper : public QObject
{
    Q_OBJECT
private:
    explicit BaseDbHelper(QObject *parent = nullptr);
    ~BaseDbHelper() override;

    QSqlDatabase m_db;                  // 数据库连接对象
    static BaseDbHelper* m_pInstance;   // 单例对象
    static QMutex m_mutex;              // 线程安全锁

public:
    // 单例获取/释放
    static BaseDbHelper* getInstance();
    static void releaseInstance();

    // 核心：连接检测+自动重连
    bool checkDbConn();

    // 密码加密（通用工具方法）
    QString encryptPwd(const QString &pwd);

    // ========== 通用SQL执行接口（所有业务层都依赖这些接口） ==========
    bool execSql(const QString &sql);
    QSqlQuery execQuery(const QString &sql);
    bool execPrepareSql(const QString &sql, const QVariantList &params);
    QSqlQuery execPrepareQuery(const QString &sql, const QVariantList &params);

    // ========== 事务接口（通用） ==========
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    bool execBatchSql(const QStringList &sqlList);
    bool execBatchPrepareSql(const QString &sql, const QList<QVariantList> &paramsList);

    // 错误信息获取
    QString getLastError();

private:
    // 读取配置文件（通用层内部处理，业务层无需关心）
    void readDbConfig();
};

#endif // BASEDBHELPER_H
