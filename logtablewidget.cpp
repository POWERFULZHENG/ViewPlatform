#include "logtablewidget.h"
#include <QSqlQuery>
#include <QSqlError>
#include "loghelper.h"
#include <QMessageBox>
#include "logdbhelper.h"

LogTableWidget::LogTableWidget(QWidget *parent) : TableOperateWidget(parent)
{
    this->setWindowTitle("系统监控");
    // ========== 执行表格初始化 ==========
    this->initTable();
    // 失能一些可以改变日志的按钮
    disabledChangeLogsBtn();
}

void LogTableWidget::disabledChangeLogsBtn()
{
    m_btnCreate->setDisabled(true);
    m_btnEdit->setDisabled(true);
    m_btnDel->setDisabled(true);
}

// 初始化日志表表头：与sys_log表字段对齐（新增日志级别、模块名称，关联用户名）
void LogTableWidget::initTableHeader()
{
    QStringList headers;
    // 调整后表头：日志ID | 用户名(关联sys_user) | 操作类型 | 操作内容 | IP地址 | 日志级别 | 模块名称 | 创建时间
    headers << "日志ID" << "用户ID" << "操作类型" << "操作内容" << "IP地址" << "日志级别" << "模块名称" << "创建时间";
    m_tableWidget->setColumnCount(headers.size());
    m_tableWidget->setHorizontalHeaderLabels(headers);

    // 可选：调整列宽（适配内容）
    m_tableWidget->setColumnWidth(0, 80);  // 日志ID
    m_tableWidget->setColumnWidth(1, 100); // 用户ID
    m_tableWidget->setColumnWidth(2, 100); // 操作类型
    m_tableWidget->setColumnWidth(3, 150);  // 操作内容
    m_tableWidget->setColumnWidth(4, 120); // IP地址
    m_tableWidget->setColumnWidth(5, 100); // 日志级别
    m_tableWidget->setColumnWidth(6, 100); // 模块名称
    m_tableWidget->setColumnWidth(7, 170); // 创建时间
}

// 加载日志数据：适配sys_log表字段 + 关联sys_user查询用户名
void LogTableWidget::loadTableData()
{
    m_tableWidget->setRowCount(0); // 清空原有数据
    LogDbHelper logDbHelper;

    // 注意：getAllLogList需调整SQL，关联sys_user表查询用户名（核心！）
    // 推荐getAllLogList的SQL：
    // SELECT l.log_id, u.username, l.operation_type, l.log_level, l.module_name, l.operation_content, l.ip_address, l.create_time
    // FROM sys_log l LEFT JOIN sys_user u ON l.user_id = u.id
    // WHERE 你的筛选条件（如UUID相关）
    QSqlQuery query;
    if(UserSession::instance()->userRole() == "超级管理员") {
        query = logDbHelper.getAllLogList();
    }else {
        query = logDbHelper.getAllLogList(UserSession::instance()->userId());
    }

    LOG_DEBUG("用户信息模块", "查询到日志条数：" << query.size());

    int row = 0;
    while(query.next())
    {
        m_tableWidget->insertRow(row);
        // 字段索引映射（与上述SQL查询字段顺序严格对应）：
        m_tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(query.value(0).toInt()))); // 0: log_id
        m_tableWidget->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));              // 1: username(操作者)
        m_tableWidget->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));              // 2: operation_type
        m_tableWidget->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));              // 3: log_level
        m_tableWidget->setItem(row, 4, new QTableWidgetItem(query.value(4).toString()));              // 4: module_name
        m_tableWidget->setItem(row, 5, new QTableWidgetItem(query.value(5).toString()));              // 5: operation_content
        m_tableWidget->setItem(row, 6, new QTableWidgetItem(query.value(6).toString()));              // 6: ip_address
        m_tableWidget->setItem(row, 7, new QTableWidgetItem(query.value(7).toString()));              // 7: create_time
        row++;
    }
}

// 新建日志（禁用，返回false）
bool LogTableWidget::slot_createNewData()
{
    return false;
}

// 编辑日志（禁用，返回false）
bool LogTableWidget::slot_editData(int selectRow)
{
    Q_UNUSED(selectRow); // 避免未使用参数警告
    return false;
}

// 删除日志（禁用，返回false）
bool LogTableWidget::slot_deleteData(int selectRow)
{
    Q_UNUSED(selectRow);
    return false;
}

// 高级筛选（预留扩展）
void LogTableWidget::slot_advancedFilter()
{
    QMessageBox::information(this, "高级筛选", "日志表高级筛选：操作类型+日志级别+模块名称+时间范围筛选\n【可扩展自定义筛选对话框】");
}

// 快速搜索筛选：适配新表结构（数据库模糊查询）
void LogTableWidget::slot_searchFilter()
{
    QString key = m_editSearch->text().trimmed();
    m_tableWidget->setRowCount(0); // 清空原有数据

    // 搜索关键词为空时，重新加载全部数据
    if(key.isEmpty()){
        loadTableData();
        return;
    }

    LogDbHelper logHelper;
    // 注意：searchLogByKey需调整SQL，关联sys_user+模糊查询多字段
    // 推荐searchLogByKey的SQL：
    // SELECT l.log_id, u.username, l.operation_type, l.log_level, l.module_name, l.operation_content, l.ip_address, l.create_time
    // FROM sys_log l LEFT JOIN sys_user u ON l.user_id = u.id
    // WHERE u.username LIKE ? OR l.operation_type LIKE ? OR l.module_name LIKE ? OR l.operation_content LIKE ?
    QSqlQuery logQuery = logHelper.searchLogByKey(key);

    int row = 0;
    while(logQuery.next())
    {
        m_tableWidget->insertRow(row);
        // 字段索引与loadTableData保持一致
        m_tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(logQuery.value(0).toInt()))); // log_id
        m_tableWidget->setItem(row, 1, new QTableWidgetItem(logQuery.value(1).toString()));              // username
        m_tableWidget->setItem(row, 2, new QTableWidgetItem(logQuery.value(2).toString()));              // operation_type
        m_tableWidget->setItem(row, 3, new QTableWidgetItem(logQuery.value(3).toString()));              // log_level
        m_tableWidget->setItem(row, 4, new QTableWidgetItem(logQuery.value(4).toString()));              // module_name
        m_tableWidget->setItem(row, 5, new QTableWidgetItem(logQuery.value(5).toString()));              // operation_content
        m_tableWidget->setItem(row, 6, new QTableWidgetItem(logQuery.value(6).toString()));              // ip_address
        m_tableWidget->setItem(row, 7, new QTableWidgetItem(logQuery.value(7).toString()));              // create_time
        row++;
    }

    // 无匹配结果时提示
    if(row == 0){
        QMessageBox::information(this, "搜索结果", QString("未找到包含关键词「%1」的日志").arg(key));
    }
}
