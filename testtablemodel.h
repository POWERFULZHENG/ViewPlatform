#ifndef TESTTABLEMODEL_H
#define TESTTABLEMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include "TestDbHelper.h"

class TestTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    // 表格列定义
    enum Column {
        ColId = 0,        // 序号
        ColTestName,      // 测试名称
        ColTestCode,      // 测试代号
        ColParamsDetail,  // 参数详情
        ColResultView,    // 结果查看
        ColMetrics,       // 指标分析
        ColExecTime,      // 执行时间
        ColEditDelete,    // 编辑/删除
        ColRemark,        // 备注
        ColCount          // 列总数
    };

    explicit TestTableModel(QObject *parent = nullptr);

    // 重写模型方法
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // 加载测试记录
    void loadTestRecords(const QList<TestRecord>& records);
    // 获取指定行的测试记录
    TestRecord getRecordAt(int row) const;
    // 删除指定行的记录
    void removeRecordAt(int row);
    // 更新指定行的记录
    void updateRecordAt(int row, const TestRecord& record);

private:
    QList<TestRecord> m_records;
    QStringList m_headerLabels = {
        "序号", "测试名称", "测试代号", "参数详情", "结果查看",
        "指标分析", "执行时间", "编辑/删除", "备注"
    };
};

#endif // TESTTABLEMODEL_H
