#ifndef LOGTABLEWIDGET_H
#define LOGTABLEWIDGET_H

#include "tableoperatewidget.h"
#include "logdbhelper.h"

class LogTableWidget : public TableOperateWidget
{
    Q_OBJECT
public:
    explicit LogTableWidget(int UUID, QWidget *parent = nullptr);
    void loadTableData() override;    // 加载用户数据

protected:
    void initTableHeader() override;  // 初始化用户表列名
    bool slot_createNewData() override;// 新建用户
    bool slot_editData(int selectRow) override;// 编辑用户
    bool slot_deleteData(int selectRow) override;// 删除用户
    void slot_advancedFilter() override;// 用户表高级筛选
    void slot_searchFilter() override;// 用户表筛选
private:
    void disabledChangeLogsBtn();
    int m_UUID;
};

#endif // LOGTABLEWIDGET_H
