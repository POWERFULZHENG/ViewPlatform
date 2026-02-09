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
    QSqlQuery searchLogByKey(const QString &key);

private:
    BaseDbHelper *m_baseDbHelper; // 依赖通用数据库层
    int m_UUID;
};

#endif // LOGDBHELPER_H
