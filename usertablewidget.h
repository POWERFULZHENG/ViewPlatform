#ifndef USERTABLEWIDGET_H
#define USERTABLEWIDGET_H

#include "tableoperatewidget.h"
#include "userdbhelper.h"

class UserTableWidget : public TableOperateWidget
{
    Q_OBJECT
public:
    explicit UserTableWidget(QWidget *parent = nullptr);

protected:
    void initTableHeader() override;  // 初始化用户表列名
    void loadTableData() override;    // 加载用户数据
    bool slot_createNewData() override;// 新建用户
    bool slot_editData(int selectRow) override;// 编辑用户
    bool slot_deleteData(int selectRow) override;// 删除用户
    void slot_advancedFilter() override;// 用户表高级筛选
    void slot_searchFilter() override;
};

#endif // USERTABLEWIDGET_H
