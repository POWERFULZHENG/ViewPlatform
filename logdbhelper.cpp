#include "LogDbHelper.h"

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
