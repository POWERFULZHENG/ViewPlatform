#include "logtablewidget.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

LogTableWidget::LogTableWidget(int UUID, QWidget *parent) : TableOperateWidget(parent)
{
    this->setWindowTitle("系统监控");
    // ========== 执行表格初始化 ==========
    this->initTable();
    // 失能一些可以改变日志的按钮
    disabledChangeLogsBtn();
    m_UUID = UUID;
}

void LogTableWidget::disabledChangeLogsBtn()
{
    m_btnCreate->setDisabled(true);
    m_btnEdit->setDisabled(true);
    m_btnDel->setDisabled(true);
}

// 初始化用户表：列名、列数
void LogTableWidget::initTableHeader()
{
    QStringList headers;
    headers << "日志ID" << "操作者" << "日志类型" << "日志内容" << "操作时间" << "ip地址";
    m_tableWidget->setColumnCount(headers.size());
    m_tableWidget->setHorizontalHeaderLabels(headers);
}

// 加载用户数据
void LogTableWidget::loadTableData()
{
    m_tableWidget->setRowCount(0);
    LogDbHelper logDbHelper;
    QSqlQuery query = logDbHelper.getAllLogList(m_UUID);
    qDebug() << "loadTableData: " << query.size();
    int row = 0;
    while(query.next())
    {
        m_tableWidget->insertRow(row);
        m_tableWidget->setItem(row,0,new QTableWidgetItem(QString::number(query.value(0).toInt())));
//        m_tableWidget->setItem(row,1,new QTableWidgetItem(query.value(1).toString()));
        m_tableWidget->setItem(row,1,new QTableWidgetItem(query.value(2).toString()));
        m_tableWidget->setItem(row,2,new QTableWidgetItem(query.value(3).toString()));
        m_tableWidget->setItem(row,3,new QTableWidgetItem(query.value(4).toString()));
        m_tableWidget->setItem(row,4,new QTableWidgetItem(query.value(5).toString()));
        m_tableWidget->setItem(row,5,new QTableWidgetItem(query.value(6).toString()));
        row++;
    }
}

// 新建用户：打开用户编辑对话框
bool LogTableWidget::slot_createNewData()
{

    return false;
}

// 编辑用户：打开对话框并赋值选中行数据
bool LogTableWidget::slot_editData(int selectRow)
{
//    todo
    return selectRow;
}

// 删除用户：业务删除逻辑
bool LogTableWidget::slot_deleteData(int selectRow)
{
//  todo
    return selectRow;
}

// 用户表高级筛选：可筛选 角色、状态、创建时间范围 等
void LogTableWidget::slot_advancedFilter()
{
    QMessageBox::information(this, "高级筛选", "用户表高级筛选：角色筛选+状态筛选+时间范围筛选\n【可扩展自定义筛选对话框】");
    // 实际项目：新建AdvancedFilterDialog，实现多条件组合筛选
}

// ========== 重写父类的快速筛选：改为【数据库模糊查询】(比前端筛选更精准高效) ==========
void LogTableWidget::slot_searchFilter()
{
    QString key = m_editSearch->text().trimmed();
    m_tableWidget->setRowCount(0);
    if(key.isEmpty()){ loadTableData(); return; }

    LogDbHelper logHelper;
    QSqlQuery logQuery = logHelper.searchLogByKey(key);
    int row=0;
    while(logQuery.next())
    {
        m_tableWidget->insertRow(row);
        m_tableWidget->setItem(row,0,new QTableWidgetItem(QString::number(logQuery.value(0).toInt())));
        m_tableWidget->setItem(row,1,new QTableWidgetItem(logQuery.value(1).toString()));
        m_tableWidget->setItem(row,2,new QTableWidgetItem(logQuery.value(2).toString()));
        m_tableWidget->setItem(row,3,new QTableWidgetItem(logQuery.value(3).toString()));
        m_tableWidget->setItem(row,4,new QTableWidgetItem(logQuery.value(4).toString()));
        m_tableWidget->setItem(row,5,new QTableWidgetItem(logQuery.value(5).toInt()==1?"启用":"禁用"));
        m_tableWidget->setItem(row,6,new QTableWidgetItem(logQuery.value(6).toString()));
        row++;
    }
}
