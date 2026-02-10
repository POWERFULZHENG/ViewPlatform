#include "LogDbHelper.h"
#include <QDebug>

LogDbHelper::LogDbHelper(QObject *parent) : QObject(parent)
{
    m_baseDbHelper = BaseDbHelper::getInstance();
}

// 获取所有日志列表
QSqlQuery LogDbHelper::getAllLogList(int UUID)
{
    QString sql = "SELECT * FROM sys_log WHERE user_id = ? ORDER BY log_id DESC";
    QSqlQuery query = m_baseDbHelper->execPrepareQuery(sql, {UUID});
    return query;
}

// 搜索日志
QSqlQuery LogDbHelper::searchLogByKey(const QString &key)
{
    QString sql = "SELECT * FROM sys_log WHERE operator LIKE ? OR operation_content LIKE ? OR operation_time LIKE ? OR ip_address LIKE ? ORDER BY log_id ASC";
    QString likeKey = "%" + key + "%";
    return m_baseDbHelper->execPrepareQuery(sql, {likeKey, likeKey, likeKey, likeKey});
}

// 插入日志：适配最终sys_log表结构
bool LogDbHelper::insertLog(int user_id,
                            const QString& operation_type,
                            const QString& operation_content,
                            const QString& ip_address,
                            const QString& log_level,
                            const QString& module_name)
{
    // 预处理SQL（仅包含最终表的字段，create_time为自动生成）
    QString sql = "INSERT INTO sys_log (user_id, operation_type, operation_content, ip_address, log_level, module_name) VALUES (?, ?, ?, ?, ?, ?)";
    return m_baseDbHelper->execPrepareSql(sql, {user_id, operation_type, operation_content, ip_address, log_level, module_name});
}
