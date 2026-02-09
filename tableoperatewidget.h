#ifndef TABLEOPERATEWIDGET_H
#define TABLEOPERATEWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QHeaderView>

class TableOperateWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TableOperateWidget(QWidget *parent = nullptr);
    ~TableOperateWidget() override = default;
    // ========== 新增这一行【核心】 ==========
    void initTable() { initTableHeader(); loadTableData(); }

protected:
    // ========== 纯虚函数【子类必须实现，业务差异化全部在这里】 ==========
    // 1. 初始化表格列名、列宽、表头
    virtual void initTableHeader() = 0;
    // 2. 加载表格业务数据（如：加载用户列表、加载日志列表）
    virtual void loadTableData() = 0;
    // 3. 新建数据-打开新建对话框，返回true表示新建成功
    virtual bool slot_createNewData() = 0;
    // 4. 编辑数据-打开编辑对话框，参数：选中行的行号，返回true表示编辑成功
    virtual bool slot_editData(int selectRow) = 0;
    // 5. 删除数据-业务删除逻辑，参数：选中行的行号/主键，返回true表示删除成功
    virtual bool slot_deleteData(int selectRow) = 0;
    // 6. 高级筛选-打开高级筛选对话框，自定义筛选条件
    virtual void slot_advancedFilter() = 0;
    // 7. 筛选-打开筛选对话框，自定义筛选条件
    virtual void slot_searchFilter();

    // ========== 通用成员变量（子类可直接访问） ==========
    QTableWidget *m_tableWidget;    // 核心表格
    QLineEdit *m_editSearch;         // 快速筛选输入框

private slots:
    // ========== 通用槽函数【子类无需重写，直接使用】 ==========
    void slot_importCsv();           // 导入CSV文件到表格
    void slot_exportCsv();           // 导出表格数据为CSV文件
    void slot_tableSelectChanged();  // 表格选中行变化，控制编辑/删除按钮禁用状态
    void slot_refreshTable();        // 刷新表格数据（通用刷新）

protected:
    // ========== 通用UI控件 ==========
    QPushButton *m_btnCreate;    // 新建
    QPushButton *m_btnEdit;      // 编辑
    QPushButton *m_btnDel;       // 删除
    QPushButton *m_btnRefresh;       // 刷新
    QPushButton *m_btnAdvFilter; // 高级筛选
    QPushButton *m_btnImport;    // 导入
    QPushButton *m_btnExport;    // 导出

    // 初始化通用UI布局（工具栏+筛选栏+表格）
    void initUiLayout();
};

#endif // TABLEOPERATEWIDGET_H
