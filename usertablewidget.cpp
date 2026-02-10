#include "usertablewidget.h"
#include "usereditdialog.h" // 新建/编辑用户的对话框，下文提供
#include <QSqlQuery>
#include <QSqlError>
#include "loghelper.h"

UserTableWidget::UserTableWidget(QWidget *parent) : TableOperateWidget(parent)
{
    this->setWindowTitle("用户信息");
    // ========== 执行表格初始化 ==========
    this->initTable();
}

// 初始化用户表：列名、列数
void UserTableWidget::initTableHeader()
{
    QStringList headers;
    // 严格对应数据库 user 表的字段顺序+字段名，前端显示别名
    headers << "用户ID" << "用户名" << "昵称" << "用户角色" << "手机号" << "密码" << "状态" << "创建时间";
    m_tableWidget->setColumnCount(headers.size());
    m_tableWidget->setHorizontalHeaderLabels(headers);
    // 列宽适配
//    m_tableWidget->setColumnWidth(0,80);m_tableWidget->setColumnWidth(1,120);m_tableWidget->setColumnWidth(5,80);
}

// 加载用户数据
void UserTableWidget::loadTableData()
{
    m_tableWidget->setRowCount(0);
    UserDbHelper userDbHelper;
    QSqlQuery query = userDbHelper.getAllUserList();
    LOG_DEBUG("用户信息模块", "用户信息数量: " << query.size());
    int row = 0;
    while(query.next())
    {
        m_tableWidget->insertRow(row);
        m_tableWidget->setItem(row,0,new QTableWidgetItem(QString::number(query.value(0).toInt())));
        m_tableWidget->setItem(row,1,new QTableWidgetItem(query.value(1).toString()));
        m_tableWidget->setItem(row,2,new QTableWidgetItem(query.value(2).toString()));
        m_tableWidget->setItem(row,3,new QTableWidgetItem(query.value(3).toString()));
        m_tableWidget->setItem(row,4,new QTableWidgetItem(query.value(4).toString()));
        m_tableWidget->setItem(row,5,new QTableWidgetItem(query.value(5).toString()));
        m_tableWidget->setItem(row,6,new QTableWidgetItem(query.value(6).toInt()==1?"启用":"禁用"));
        m_tableWidget->setItem(row,7,new QTableWidgetItem(query.value(7).toString()));
//        m_tableWidget->setItem(row,8,new QTableWidgetItem(query.value(8).toString()));
        row++;
    }
}

// 新建用户：打开用户编辑对话框
bool UserTableWidget::slot_createNewData()
{
    UserEditDialog dlg(this, UserEditDialog::Oper_Create);
    qDebug() << "slot_createNewData" ;
    return dlg.exec() == QDialog::Accepted;
}

// 编辑用户：打开对话框并赋值选中行数据
bool UserTableWidget::slot_editData(int selectRow)
{
    if(selectRow <0) return false;
    // 获取选中行的用户数据
    int userId = m_tableWidget->item(selectRow,0)->text().toInt();
    QString userName = m_tableWidget->item(selectRow,1)->text();
    QString nickName = m_tableWidget->item(selectRow,2)->text();
    QString roleName = m_tableWidget->item(selectRow,3)->text();
    QString phone = m_tableWidget->item(selectRow,4)->text();
    QString pwd = m_tableWidget->item(selectRow,5)->text();
    int status = m_tableWidget->item(selectRow,6)->text()=="启用"?1:0;

    UserEditDialog dlg(this, UserEditDialog::Oper_Edit);
    dlg.setFormData(userId,userName,nickName,roleName,phone,pwd,status);
    return dlg.exec() == QDialog::Accepted;
}

// 删除用户：业务删除逻辑
bool UserTableWidget::slot_deleteData(int selectRow)
{
    int userId = m_tableWidget->item(selectRow,0)->text().toInt();
    UserDbHelper userDbHelper;
    bool res = userDbHelper.delUserById(userId);
    if(res)
    {
        QMessageBox::information(this,"成功","用户删除成功！");
    }
    else
    {
        QMessageBox::critical(this,"删除失败","数据库执行失败！");
    }
    return res;
}

// 用户表高级筛选：可筛选 角色、状态、创建时间范围 等【待改】
void UserTableWidget::slot_advancedFilter()
{
    QMessageBox::information(this,"待开发","待开发，耐心等待！");
//    // 示例：多条件组合筛选，可扩展为自定义筛选对话框，此处为核心SQL逻辑
//    QString sql = "SELECT * FROM sys_user WHERE 1=1 ";
//    // 可追加条件：比如 角色=普通用户 + 状态=启用 + 创建时间>2026-01-01
//    sql += " AND role_name='普通用户' AND status=1 AND create_time > '2026-01-01'";
//    sql += " ORDER BY id DESC";

//    m_tableWidget->setRowCount(0);
//    MysqlHelper *dbHelper = MysqlHelper::getInstance();
//    // 示例而已，还需要更改
//    QSqlQuery query = dbHelper->searchUserByKey(sql);
//    int row=0;
//    while(query.next()){
//        m_tableWidget->insertRow(row);
//        m_tableWidget->setItem(row,0,new QTableWidgetItem(QString::number(query.value(0).toInt())));
//        m_tableWidget->setItem(row,1,new QTableWidgetItem(query.value(1).toString()));
//        m_tableWidget->setItem(row,2,new QTableWidgetItem(query.value(2).toString()));
//        m_tableWidget->setItem(row,3,new QTableWidgetItem(query.value(3).toString()));
//        m_tableWidget->setItem(row,4,new QTableWidgetItem(query.value(4).toString()));
//        m_tableWidget->setItem(row,5,new QTableWidgetItem(query.value(5).toInt()==1?"启用":"禁用"));
//        m_tableWidget->setItem(row,6,new QTableWidgetItem(query.value(6).toString()));
//        row++;
//    }
}

// ========== 重写父类的快速筛选：改为【数据库模糊查询】(比前端筛选更精准高效) ==========
void UserTableWidget::slot_searchFilter()
{
    QString key = m_editSearch->text().trimmed();
    m_tableWidget->setRowCount(0);
    if(key.isEmpty()){ loadTableData(); return; }

    UserDbHelper userDbHelper;
    QSqlQuery query = userDbHelper.searchUserByKey(key);
    int row=0;
    while(query.next())
    {
        m_tableWidget->insertRow(row);
        m_tableWidget->setItem(row,0,new QTableWidgetItem(QString::number(query.value(0).toInt())));
        m_tableWidget->setItem(row,1,new QTableWidgetItem(query.value(1).toString()));
        m_tableWidget->setItem(row,2,new QTableWidgetItem(query.value(2).toString()));
        m_tableWidget->setItem(row,3,new QTableWidgetItem(query.value(3).toString()));
        m_tableWidget->setItem(row,4,new QTableWidgetItem(query.value(4).toString()));
        m_tableWidget->setItem(row,5,new QTableWidgetItem(query.value(5).toString()));
        m_tableWidget->setItem(row,6,new QTableWidgetItem(query.value(6).toInt()==1?"启用":"禁用"));
        m_tableWidget->setItem(row,7,new QTableWidgetItem(query.value(7).toString()));
        row++;
    }
}
