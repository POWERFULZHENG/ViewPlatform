#include "TestTableModel.h"
#include <QDateTime>
#include <QDebug>

TestTableModel::TestTableModel(QObject *parent) : QAbstractTableModel(parent) {
}

int TestTableModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_records.size();
}

int TestTableModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return ColCount;
}

QVariant TestTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_records.size()) {
        return QVariant();
    }

    const TestRecord& record = m_records[index.row()];
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case ColId:
            return record.test_id;
        case ColTestName:
            return record.test_name;
        case ColTestCode:
            return record.test_code;
        case ColParamsDetail:
            return "查看详情"; // 显示按钮文本
        case ColResultView:
            return "打开文件夹"; // 显示按钮文本
        case ColMetrics:
            return "查看指标"; // 显示按钮文本
        case ColExecTime:
            return record.execute_time.toString("yyyy-MM-dd HH:mm:ss");
        case ColEditDelete:
            return "编辑/删除"; // 显示按钮文本
        case ColRemark:
            return record.remark;
        default:
            return QVariant();
        }
    } else if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }

    return QVariant();
}

QVariant TestTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section < m_headerLabels.size()) {
            return m_headerLabels[section];
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

bool TestTableModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || index.row() >= m_records.size() || role != Qt::EditRole) {
        return false;
    }

    TestRecord& record = m_records[index.row()];
    switch (index.column()) {
    case ColTestName:
        record.test_name = value.toString();
        break;
    case ColTestCode:
        record.test_code = value.toString();
    case ColRemark:
        record.remark = value.toString();
        break;
    default:
        return false;
    }

    emit dataChanged(index, index);
    return true;
}

Qt::ItemFlags TestTableModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    // 允许编辑测试名称、测试代号、备注列
    if (index.column() == ColTestName || index.column() == ColTestCode || index.column() == ColRemark) {
        flags |= Qt::ItemIsEditable;
    }
    return flags;
}

void TestTableModel::loadTestRecords(const QList<TestRecord>& records) {
    beginResetModel();
    m_records = records;
    endResetModel();
}

TestRecord TestTableModel::getRecordAt(int row) const {
    if (row >= 0 && row < m_records.size()) {
        return m_records[row];
    }
    return TestRecord();
}

void TestTableModel::removeRecordAt(int row) {
    if (row >= 0 && row < m_records.size()) {
        beginRemoveRows(QModelIndex(), row, row);
        m_records.removeAt(row);
        endRemoveRows();
    }
}

void TestTableModel::updateRecordAt(int row, const TestRecord& record) {
    if (row >= 0 && row < m_records.size()) {
        m_records[row] = record;
        emit dataChanged(index(row, 0), index(row, ColCount-1));
    }
}
