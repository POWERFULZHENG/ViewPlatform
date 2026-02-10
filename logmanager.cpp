#include "logmanager.h"
#include "logdbhelper.h"
#include <QDebug>

LogManager* LogManager::m_instance = nullptr;
QMutex LogManager::m_mutex;

LogManager* LogManager::getInstance()
{
    if (m_instance == nullptr) {
        m_mutex.lock();
        if (m_instance == nullptr) {
            m_instance = new LogManager();
        }
        m_mutex.unlock();
    }
    return m_instance;
}

LogManager::LogManager() {}

// 扩展接口实现：移除operatorName参数
void LogManager::addLog(const QString& moduleName,
                        const QString& operation,
                        LogLevel level,
                        int user_id,
                        const QString& operation_type,
                        const QString& ip_address)
{
    QDateTime logTime = QDateTime::currentDateTime();
    bool success = saveLogToDb(moduleName, operation, level, user_id, operation_type, ip_address, logTime);
    if (!success) {
        qWarning() << QString("日志保存失败：模块[%1] 用户ID[%2]").arg(moduleName).arg(user_id);
    }
}

// 数据库保存逻辑：适配最终表字段
bool LogManager::saveLogToDb(const QString& moduleName,
                             const QString& operation,
                             LogLevel level,
                             int user_id,
                             const QString& operation_type,
                             const QString& ip_address,
                             const QDateTime& logTime)
{
    LogDbHelper dbHelper;

    // 日志级别转换（与表中log_level值对应）
    QString levelStr;
    switch (level) {
    case Info: levelStr = "INFO"; break;
    case Warning: levelStr = "WARNING"; break;
    case Error: levelStr = "ERROR"; break;
    case Debug: levelStr = "DEBUG"; break;
    default: levelStr = "INFO";
    }

    // 调用数据库层插入方法
    return dbHelper.insertLog(user_id,
                              operation_type,
                              operation,
                              ip_address,
                              levelStr,
                              moduleName);
}
