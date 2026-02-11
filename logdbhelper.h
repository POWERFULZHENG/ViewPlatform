#ifndef LOGDBHELPER_H
#define LOGDBHELPER_H

#include <QObject>
#include <QSqlQuery>
#include "BaseDbHelper.h"

// 日志业务助手：仅处理日志相关业务逻辑
class LogDbHelper : public QObject
{
    Q_OBJECT
public:
    explicit LogDbHelper(QObject *parent = nullptr);

    // 日志查询操作
    QSqlQuery getAllLogList(int UUID);
    QSqlQuery getAllLogList();
    QSqlQuery searchLogByKey(const QString &key);
    // 插入日志：参数顺序与最终sys_log表字段对齐
    bool insertLog(int user_id,                     // 关联用户ID
                   const QString& operation_type,   // 操作类型
                   const QString& operation_content,// 操作内容
                   const QString& ip_address,       // IP地址（IPv4/IPv6）
                   const QString& log_level,        // 日志级别
                   const QString& module_name);     // 模块名称
private:
    BaseDbHelper *m_baseDbHelper; // 依赖通用数据库层
};

#endif // LOGDBHELPER_H
