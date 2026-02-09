#include "tableoperatewidget.h"
#include <QDebug>

TableOperateWidget::TableOperateWidget(QWidget *parent) : QWidget(parent)
{
    initUiLayout();  // 初始化通用UI
}

void TableOperateWidget::initUiLayout()
{
    // 1. 创建核心控件
    m_tableWidget = new QTableWidget(this);
    m_editSearch = new QLineEdit(this);
    m_btnCreate = new QPushButton("新建", this);
    m_btnEdit = new QPushButton("编辑", this);
    m_btnDel = new QPushButton("删除", this);
    m_btnRefresh = new QPushButton("刷新", this);
    QPushButton *btnSearch = new QPushButton("筛选", this);
    m_btnAdvFilter = new QPushButton("高级筛选", this);
    m_btnImport = new QPushButton("导入", this);
    m_btnExport = new QPushButton("导出", this);

    // 2. 表格属性配置【通用】
    m_tableWidget->setSelectionBehavior(QTableWidget::SelectRows);  // 整行选中
    m_tableWidget->setSelectionMode(QTableWidget::SingleSelection); // 单选模式
    m_tableWidget->setEditTriggers(QTableWidget::NoEditTriggers);   // 表格禁止直接编辑，仅通过对话框编辑
    m_tableWidget->setAlternatingRowColors(true);                   // 隔行变色，提升可读性
    m_tableWidget->horizontalHeader()->setStretchLastSection(true); // 最后一列自适应宽度
    m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // 列宽适配内容

    // 3. 筛选框提示文字
    m_editSearch->setPlaceholderText("输入关键词，全局模糊筛选...");

    // 4. 按钮状态初始化：默认编辑/删除禁用（无选中行）
    m_btnEdit->setEnabled(false);
    m_btnDel->setEnabled(false);

    // ========== 布局组装 ==========
    // 工具栏布局（功能按钮）
    QHBoxLayout *hlayout_tool = new QHBoxLayout;
    hlayout_tool->addWidget(m_btnCreate);
    hlayout_tool->addWidget(m_btnEdit);
    hlayout_tool->addWidget(m_btnDel);
    hlayout_tool->addWidget(m_btnRefresh);
    hlayout_tool->addStretch();
    hlayout_tool->addWidget(m_btnAdvFilter);
    hlayout_tool->addWidget(m_btnImport);
    hlayout_tool->addWidget(m_btnExport);

    // 筛选栏布局
    QHBoxLayout *hlayout_search = new QHBoxLayout;
    hlayout_search->addWidget(new QLabel("快速筛选：", this));
    hlayout_search->addWidget(m_editSearch);
    hlayout_search->addWidget(btnSearch);

    // 主布局
    QVBoxLayout *vlayout_main = new QVBoxLayout(this);
    vlayout_main->addLayout(hlayout_tool);
    vlayout_main->addLayout(hlayout_search);
    vlayout_main->addWidget(m_tableWidget);
    vlayout_main->setSpacing(10);
    vlayout_main->setContentsMargins(10,10,10,10);
    this->setLayout(vlayout_main);

    // ========== 通用信号槽绑定 ==========
    connect(m_btnCreate, &QPushButton::clicked, this, [=]{
        qDebug() << "slot_createNewData1" ;
        if(slot_createNewData()) slot_refreshTable(); // 新建成功则刷新表格
    });
    connect(m_btnEdit, &QPushButton::clicked, this, [=]{
        int row = m_tableWidget->currentRow();
        if(row >=0 && slot_editData(row)) slot_refreshTable(); // 编辑成功则刷新表格
    });
    connect(m_btnDel, &QPushButton::clicked, this, [=]{
        int row = m_tableWidget->currentRow();
        if(row <0) return;
        if(QMessageBox::question(this, "确认删除", "是否确定删除选中数据？删除后不可恢复！") != QMessageBox::Yes) return;
        if(slot_deleteData(row)) slot_refreshTable(); // 删除成功则刷新表格
    });
    connect(m_btnRefresh, &QPushButton::clicked, this, &TableOperateWidget::slot_refreshTable);
    connect(btnSearch, &QPushButton::clicked, this, &TableOperateWidget::slot_searchFilter);
    connect(m_btnAdvFilter, &QPushButton::clicked, this, &TableOperateWidget::slot_advancedFilter);
    connect(m_btnImport, &QPushButton::clicked, this, &TableOperateWidget::slot_importCsv);
    connect(m_btnExport, &QPushButton::clicked, this, &TableOperateWidget::slot_exportCsv);
    connect(m_tableWidget, &QTableWidget::currentCellChanged, this, &TableOperateWidget::slot_tableSelectChanged);
    connect(m_editSearch, &QLineEdit::returnPressed, this, &TableOperateWidget::slot_searchFilter);
}

// 快速筛选：全局模糊匹配表格所有单元格内容【通用，适配所有表格】
void TableOperateWidget::slot_searchFilter()
{
    QString key = m_editSearch->text().trimmed();
    for(int row=0; row<m_tableWidget->rowCount(); row++)
    {
        bool isMatch = false;
        for(int col=0; col<m_tableWidget->columnCount(); col++)
        {
            QTableWidgetItem *item = m_tableWidget->item(row, col);
            if(item && item->text().contains(key, Qt::CaseInsensitive))
            {
                isMatch = true;
                break;
            }
        }
        m_tableWidget->setRowHidden(row, !isMatch);
    }
}

// 导入CSV文件【通用，适配所有表格】
void TableOperateWidget::slot_importCsv()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择CSV文件", "", "CSV文件 (*.csv);;所有文件 (*.*)");
    if(filePath.isEmpty()) return;

    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "导入失败", "文件打开失败，请检查文件是否被占用！");
        return;
    }

    m_tableWidget->setRowCount(0); // 清空原有数据
    QTextStream in(&file);
    in.setCodec("UTF-8"); // 解决中文乱码

    int row = 0;
    while(!in.atEnd())
    {
        QString line = in.readLine();
        QStringList dataList = line.split(","); // CSV逗号分隔
        m_tableWidget->insertRow(row);
        for(int col=0; col<dataList.size() && col<m_tableWidget->columnCount(); col++)
        {
            m_tableWidget->setItem(row, col, new QTableWidgetItem(dataList.at(col)));
        }
        row++;
    }
    file.close();
    QMessageBox::information(this, "导入成功", QString("共导入 %1 条数据").arg(row));
}

// 导出CSV文件【通用，适配所有表格，默认带时间戳，Excel可直接打开】
void TableOperateWidget::slot_exportCsv()
{
    QString fileName = QString("%1_%2.csv").arg(this->windowTitle()).arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
    QString filePath = QFileDialog::getSaveFileName(this, "导出CSV文件", fileName, "CSV文件 (*.csv);;所有文件 (*.*)");
    if(filePath.isEmpty()) return;

    QFile file(filePath);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "导出失败", "文件保存失败，请检查权限！");
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    // 导出表头
    for(int col=0; col<m_tableWidget->columnCount(); col++)
    {
        out << m_tableWidget->horizontalHeaderItem(col)->text() << ",";
    }
    out << "\n";

    // 导出表格数据
    for(int row=0; row<m_tableWidget->rowCount(); row++)
    {
        for(int col=0; col<m_tableWidget->columnCount(); col++)
        {
            QTableWidgetItem *item = m_tableWidget->item(row, col);
            out << (item ? item->text() : "") << ",";
        }
        out << "\n";
    }
    file.close();
    QMessageBox::information(this, "导出成功", "数据已成功导出为CSV文件！");
}

// 表格选中行变化：控制编辑/删除按钮状态【通用】
void TableOperateWidget::slot_tableSelectChanged()
{
    int selectRow = m_tableWidget->currentRow();
    m_btnEdit->setEnabled(selectRow >= 0);
    m_btnDel->setEnabled(selectRow >= 0);
}

// 刷新表格【通用】
void TableOperateWidget::slot_refreshTable()
{
    m_editSearch->clear();
    m_tableWidget->setRowCount(0);
    loadTableData();
}
