#ifndef MYSQLHELPER_H
#define MYSQLHELPER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QMutex>
#include <QString>
#include <QVariant>

class MysqlHelper : public QObject
{
    Q_OBJECT
private:
    // 私有构造+析构，禁止外部实例化
    explicit MysqlHelper(QObject *parent = nullptr);
    ~MysqlHelper() override;

    // 单例核心成员
    static MysqlHelper* m_pInstance;
    static QMutex       m_mutex;      // 线程安全锁
    QSqlDatabase        m_db;         // 数据库连接对象

    // 密码加密：简单MD5加密（Qt自带，无需第三方库，生产可用）
    QString encryptPwd(const QString &pwd);

    // 通用检测连接+自动重连 核心函数【所有数据库操作前置调用，根治未打开问题】
    bool checkDbConn();

    // ========== 事务相关接口（新增） ==========
    bool beginTransaction();  // 开始事务
    bool commitTransaction(); // 提交事务
    bool rollbackTransaction();// 回滚事务

    // 批量操作接口（原子性保障）
    bool execBatchSql(const QStringList &sqlList); // 批量执行普通SQL
    bool execBatchPrepareSql(const QString &sql, const QList<QVariantList> &paramsList); // 批量预处理SQL

public:
    // 线程安全的单例获取接口（全局唯一实例）
    static MysqlHelper *getInstance();
    // 单例内存释放接口（程序退出时调用，防止内存泄漏）
    static void releaseInstance();

    // ========== 通用数据库接口【所有业务共用，减少冗余】 ==========
    bool execSql(const QString &sql);                  // 执行增/删/改 SQL
    QSqlQuery execQuery(const QString &sql);           // 执行查询 SQL
    bool execPrepareSql(const QString &sql, const QVariantList &params); // 预处理执行SQL（防注入）
    QSqlQuery execPrepareQuery(const QString &sql, const QVariantList &params); // 预处理查询SQL
    QString getLastError() const;

    // ========== 用户表 专属数据库接口【纯数据库操作，无业务逻辑，无UI】 ==========
    bool userRegister(const QString &phone, const QString &pwd);
    bool userLogin(const QString &phone, const QString &pwd);
    bool checkPhoneExist(const QString &phone);
    bool modifyUserPwd(const QString &phone, const QString &oldPwd, const QString &newPwd);
    QString getUserInfoByPhone(const QString &phone);
    bool delUserById(int userId);
    bool addUser(const QString &userName, const QString &nickName, const QString &roleName, const QString &phone, int status);
    bool updateUser(int userId, const QString &userName, const QString &nickName, const QString &roleName, const QString &phone, int status);
    QSqlQuery getAllUserList();
    QSqlQuery searchUserByKey(const QString &key);

    // ========== 日志表 专属数据库接口 ==========
    QSqlQuery getAllLogList();
    QSqlQuery searchLogByKey(const QString &key);
};

#endif // MYSQLHELPER_H
