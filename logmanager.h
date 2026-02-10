#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QString>
#include <QDateTime>
#include <QMutex>

class LogManager
{
public:
    // 日志级别枚举（与表中log_level字段对应）
    enum LogLevel {
        Info,    // INFO
        Warning, // WARNING
        Error,   // ERROR
        Debug    // DEBUG
    };
public:
    static LogManager* getInstance();

    // 核心接口：适配最终sys_log表字段
    void addLog(const QString& moduleName,  // 模块名称（module_name）
                const QString& operation,   // 操作内容（operation_content）
                LogLevel level = Info,      // 日志级别（log_level）
                int user_id = 0,            // 关联用户ID（user_id，非空）
                const QString& operation_type = "", // 操作类型（operation_type）
                const QString& ip_address = ""); // IP地址（ip_address，支持IPv4/IPv6）

private:
    LogManager();
    ~LogManager() = default;
    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&) = delete;

    // 底层数据库保存方法：参数与最终表对齐
    bool saveLogToDb(const QString& moduleName,
                     const QString& operation,
                     LogLevel level,
                     int user_id,
                     const QString& operation_type,
                     const QString& ip_address,
                     const QDateTime& logTime);

    static LogManager* m_instance;
    static QMutex m_mutex;
};

// 简化调用宏（适配新接口）
#define ADD_LOG(module, op, level, user_id, op_type, ip) LogManager::getInstance()->addLog(module, op, level, user_id, op_type, ip)
// 常用信息日志宏（默认级别Info）
#define ADD_BASE_LOG(module, op, user_id, op_type, ip) ADD_LOG(module, op, LogManager::Info, user_id, op_type, ip)

#endif // LOGMANAGER_H
