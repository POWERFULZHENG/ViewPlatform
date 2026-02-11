#ifndef CONFIGWIDGET_H
#define CONFIGWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include "TestDbHelper.h"
#include "testtablemodel.h"
#include "pythonrunner.h"
#include <QString>
#include <QJsonDocument>

namespace Ui {
class ConfigWidget; // 对应Qt Designer设计的UI文件
}

class ConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigWidget(TestTableModel *testTableModel, QWidget *parent = nullptr);
    ~ConfigWidget();

    // 加载配置参数到界面
    void loadConfigParams(const ConfigParams& params);
    // 获取界面上的配置参数
    ConfigParams getConfigParams() const;

    // 锁定/解锁编辑
    void setEditLocked(bool locked);

private slots:
     // 配置确认后的处理
     void onConfigConfirmed(const ConfigParams& params);
     // Python脚本执行完成
     void onPythonScriptFinished(bool success, const QDateTime& execTime, const QString& metricsData);
     // Python日志输出
     void onPythonLogOutput(const QString& log);
     // 表格点击事件
     void onTableViewClicked(const QModelIndex& index);
     // 加载测试记录
     void loadTestRecords();
     // 显示参数详情弹窗
     void showParamsDetailDialog(const QString& paramsJson);
     // 显示指标分析弹窗
     void showMetricsDialog(const QString& metricsJson);
     // 编辑/删除测试记录弹窗
     void showEditDeleteDialog(int row, const TestRecord& record);
     void onBtnStartAlgorithmClicked(); // 启动算法
     void onBtnInterruptClicked();      // 中断算法
     void onBtnResetDefaultClicked();   // 恢复默认配置
     void onBtnSelectScriptClicked();   // 选择执行脚本

signals:
    // 确定按钮点击（传递配置参数）
    void confirmConfig(const ConfigParams& params);
    // 取消按钮点击
    void cancelConfig();

private slots:
    void on_btnConfirm_clicked();
    void on_btnCancel_clicked();

private:
    Ui::ConfigWidget *ui;
    bool m_isLocked = false;

    TestTableModel *m_testTableModel;      // 测试结果表格模型
    PythonRunner *m_pythonRunner;          // Python脚本运行器
    TestDbHelper *m_testDbHelper;          // 数据库操作类
    // 临时变量
    int m_currentConfigId;                 // 当前配置ID
    QString m_currentTestName;             // 当前测试名称
    QString m_currentTestCode;             // 当前测试代号
public:
    // 初始化UI控件
    void initUI();
    // 绑定控件和参数的映射
    void bindParamsToUI();
    QWidget* getTestTableWidget();
};

#endif // CONFIGWIDGET_H
