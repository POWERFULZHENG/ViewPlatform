#include "algorithmsdatawidget.h"
#include <QDesktopServices>
#include <QUrl>
#include <QDialog>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QJsonDocument>
#include <QDebug>

AlgorithmsDataWidget::AlgorithmsDataWidget(Ui::MainWindow* ui, QMainWindow *mainWindow, QWidget *parent)
    : QWidget(parent)
    , m_ui(ui)
    , m_mainWindow(mainWindow)
{
    // 初始化模块
    m_testDbHelper = TestDbHelper::getInstance();
    m_pythonRunner = new PythonRunner(m_mainWindow);
    m_configWidget = new ConfigWidget(m_mainWindow);
    m_testTableModel = new TestTableModel(m_mainWindow);

    // 将配置页面添加到主窗口
    m_ui->widgetConfig->layout()->addWidget(m_configWidget);

    // 设置表格模型
    ui->tableViewTest->setModel(m_testTableModel);
    // 调整列宽
    ui->tableViewTest->horizontalHeader()->setStretchLastSection(true);
    ui->tableViewTest->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // 加载测试记录
    loadTestRecords();

    // 绑定配置页面信号
    connect(m_configWidget, &ConfigWidget::confirmConfig, this, &AlgorithmsDataWidget::onConfigConfirmed);
    // 绑定Python Runner信号
    connect(m_pythonRunner, &PythonRunner::finished, this, &AlgorithmsDataWidget::onPythonScriptFinished);
    connect(m_pythonRunner, &PythonRunner::logOutput, this, &AlgorithmsDataWidget::onPythonLogOutput);
    // 绑定表格点击事件
    connect(ui->tableViewTest, &QTableView::clicked, this, &AlgorithmsDataWidget::onTableViewClicked);
}

AlgorithmsDataWidget::~AlgorithmsDataWidget() {
    delete ui;
}

void AlgorithmsDataWidget::loadTestRecords() {
    QList<TestRecord> records = m_testDbHelper->getAllTestRecords();
    m_testTableModel->loadTestRecords(records);
}

void AlgorithmsDataWidget::onConfigConfirmed(const ConfigParams& params) {
    // 保存配置参数到数据库
    int configId = -1;
    if (!m_testDbHelper->saveConfigParams(params, configId)) {
        QMessageBox::critical(this, "错误", "保存配置参数到数据库失败！");
        return;
    }

    // 获取测试名称和代号（可从UI输入框获取，此处示例）
    QString testName = ui->lineEditTestName->text().trimmed();
    QString testCode = ui->lineEditTestCode->text().trimmed();
    if (testName.isEmpty() || testCode.isEmpty()) {
        QMessageBox::warning(this, "警告", "测试名称和代号不能为空！");
        m_configWidget->setEditLocked(false); // 解锁编辑
        return;
    }

    // 转换配置参数为JSON字符串
    QJsonObject paramsJson = params.toJson();
    QString paramsDetail = QJsonDocument(paramsJson).toJson(QJsonDocument::Indented);

    // 保存测试记录（先保存基础信息，待Python执行完成后更新结果）
    TestRecord record;
    record.test_name = testName;
    record.test_code = testCode;
    record.params_detail = paramsDetail;
    record.config_id = configId;

    if (!m_testDbHelper->addTestRecord(record)) {
        QMessageBox::critical(this, "错误", "保存测试记录失败！");
        m_configWidget->setEditLocked(false); // 解锁编辑
        return;
    }

    // 启动Python脚本
    m_currentConfigId = configId;
    m_currentTestName = testName;
    m_currentTestCode = testCode;

    m_pythonRunner->setScriptPath(ui->lineEditPythonScript->text().trimmed()); // Python脚本路径从UI输入
    m_pythonRunner->setScriptParams(paramsJson);
    if (!m_pythonRunner->start()) {
        QMessageBox::critical(this, "错误", "启动Python脚本失败！");
        m_configWidget->setEditLocked(false); // 解锁编辑
    }
}

void AlgorithmsDataWidget::onPythonScriptFinished(bool success, const QDateTime& execTime, const QString& metricsData) {
    // 获取最后添加的测试记录（根据测试名称和代号）
    QList<TestRecord> records = m_testDbHelper->getAllTestRecords();
    TestRecord targetRecord;
    for (const TestRecord& record : records) {
        if (record.test_name == m_currentTestName && record.test_code == m_currentTestCode && record.config_id == m_currentConfigId) {
            targetRecord = record;
            break;
        }
    }

    if (targetRecord.test_id == -1) {
        QMessageBox::warning(this, "警告", "未找到对应的测试记录！");
        return;
    }

    // 更新测试记录
    targetRecord.execute_time = execTime;
    targetRecord.metrics_data = metricsData;
    targetRecord.result_path = m_pythonRunner->getResultPath();

    if (success) {
        m_testDbHelper->updateTestRecord(targetRecord);
        QMessageBox::information(this, "成功", "Python脚本执行完成，结果已保存！");
    } else {
        targetRecord.remark = "执行失败";
        m_testDbHelper->updateTestRecord(targetRecord);
        QMessageBox::critical(this, "错误", "Python脚本执行失败！");
    }

    // 重新加载测试记录
    loadTestRecords();
}

void AlgorithmsDataWidget::onPythonLogOutput(const QString& log) {
    // 将日志输出到UI的文本框
    ui->textEditLog->append(log);
}

void AlgorithmsDataWidget::onTableViewClicked(const QModelIndex& index) {
    if (!index.isValid()) {
        return;
    }

    int row = index.row();
    TestRecord record = m_testTableModel->getRecordAt(row);

    switch (index.column()) {
    case TestTableModel::ColParamsDetail:
        // 显示参数详情弹窗
        showParamsDetailDialog(record.params_detail);
        break;
    case TestTableModel::ColResultView:
        // 打开结果文件夹
        if (!record.result_path.isEmpty()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(record.result_path));
        } else {
            QMessageBox::warning(this, "警告", "结果文件夹路径为空！");
        }
        break;
    case TestTableModel::ColMetrics:
        // 显示指标分析弹窗
        showMetricsDialog(record.metrics_data);
        break;
    case TestTableModel::ColEditDelete:
        // 编辑/删除弹窗
        showEditDeleteDialog(row, record);
        break;
    default:
        break;
    }
}

void AlgorithmsDataWidget::showParamsDetailDialog(const QString& paramsJson) {
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("参数详情");
    dialog->resize(800, 600);

    QTextEdit* textEdit = new QTextEdit(dialog);
    textEdit->setReadOnly(true);
    textEdit->setText(paramsJson);

    QPushButton* btnClose = new QPushButton("关闭", dialog);
    connect(btnClose, &QPushButton::clicked, dialog, &QDialog::close);

    QVBoxLayout* layout = new QVBoxLayout(dialog);
    layout->addWidget(textEdit);
    layout->addWidget(btnClose, 0, Qt::AlignRight);

    dialog->exec();
    dialog->deleteLater();
}

void AlgorithmsDataWidget::showMetricsDialog(const QString& metricsJson) {
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("指标分析");
    dialog->resize(600, 400);

    QTextEdit* textEdit = new QTextEdit(dialog);
    textEdit->setReadOnly(true);
    if (metricsJson.isEmpty()) {
        textEdit->setText("暂无指标数据");
    } else {
        // 格式化JSON显示
        QJsonDocument doc = QJsonDocument::fromJson(metricsJson.toUtf8());
        textEdit->setText(doc.toJson(QJsonDocument::Indented));
    }

    QPushButton* btnClose = new QPushButton("关闭", dialog);
    connect(btnClose, &QPushButton::clicked, dialog, &QDialog::close);

    QVBoxLayout* layout = new QVBoxLayout(dialog);
    layout->addWidget(textEdit);
    layout->addWidget(btnClose, 0, Qt::AlignRight);

    dialog->exec();
    dialog->deleteLater();
}

void AlgorithmsDataWidget::showEditDeleteDialog(int row, const TestRecord& record) {
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("编辑/删除测试记录");
    dialog->resize(400, 300);

    // 编辑控件
    QLineEdit* lineEditTestName = new QLineEdit(dialog);
    lineEditTestName->setPlaceholderText("测试名称");
    lineEditTestName->setText(record.test_name);

    QLineEdit* lineEditTestCode = new QLineEdit(dialog);
    lineEditTestCode->setPlaceholderText("测试代号");
    lineEditTestCode->setText(record.test_code);

    QTextEdit* textEditRemark = new QTextEdit(dialog);
    textEditRemark->setPlaceholderText("备注");
    textEditRemark->setText(record.remark);

    // 按钮
    QPushButton* btnSave = new QPushButton("保存", dialog);
    QPushButton* btnDelete = new QPushButton("删除", dialog);
    QPushButton* btnCancel = new QPushButton("取消", dialog);

    connect(btnSave, &QPushButton::clicked, [=]() {
        TestRecord updatedRecord = record;
        updatedRecord.test_name = lineEditTestName->text().trimmed();
        updatedRecord.test_code = lineEditTestCode->text().trimmed();
        updatedRecord.remark = textEditRemark->toPlainText().trimmed();

        if (m_testDbHelper->updateTestRecord(updatedRecord)) {
            m_testTableModel->updateRecordAt(row, updatedRecord);
            QMessageBox::information(this, "成功", "测试记录更新成功！");
            dialog->close();
        } else {
            QMessageBox::critical(this, "错误", "测试记录更新失败！");
        }
    });

    connect(btnDelete, &QPushButton::clicked, [=]() {
        if (QMessageBox::question(this, "确认", "确定要删除该测试记录吗？") == QMessageBox::Yes) {
            if (m_testDbHelper->deleteTestRecord(record.test_id)) {
                m_testTableModel->removeRecordAt(row);
                QMessageBox::information(this, "成功", "测试记录删除成功！");
                dialog->close();
            } else {
                QMessageBox::critical(this, "错误", "测试记录删除失败！");
            }
        }
    });

    connect(btnCancel, &QPushButton::clicked, dialog, &QDialog::close);

    // 布局
    QVBoxLayout* vLayout = new QVBoxLayout(dialog);
    vLayout->addWidget(new QLabel("测试名称："));
    vLayout->addWidget(lineEditTestName);
    vLayout->addWidget(new QLabel("测试代号："));
    vLayout->addWidget(lineEditTestCode);
    vLayout->addWidget(new QLabel("备注："));
    vLayout->addWidget(textEditRemark);

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addWidget(btnSave);
    hLayout->addWidget(btnDelete);
    hLayout->addWidget(btnCancel);

    vLayout->addLayout(hLayout);

    dialog->exec();
    dialog->deleteLater();
}
