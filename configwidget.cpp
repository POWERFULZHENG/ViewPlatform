#include "ConfigWidget.h"
#include "ui_ConfigWidget.h"
#include <QDebug>
#include <QMessageBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QTableView>
#include <QHeaderView>

ConfigWidget::ConfigWidget(TestTableModel *testTableModel, int UUID, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfigWidget)
{
    ui->setupUi(this);
    initUI();
    bindParamsToUI(); // 此处为空，仅保留接口，实际映射在load/get中完成

    m_testDbHelper = TestDbHelper::getInstance();
    m_pythonRunner = new PythonRunner(this);
    m_testTableModel = testTableModel;
    m_UUID = UUID;

    // 绑定信号槽
    connect(ui->btnStartAlgorithm, &QPushButton::clicked, this, &ConfigWidget::onBtnStartAlgorithmClicked);
    connect(ui->btnInterrupt, &QPushButton::clicked, this, &ConfigWidget::onBtnInterruptClicked);
    connect(ui->btnResetDefault, &QPushButton::clicked, this, &ConfigWidget::onBtnResetDefaultClicked);
    connect(ui->btnSelectScript, &QPushButton::clicked, this, &ConfigWidget::onBtnSelectScriptClicked);

    // 绑定Python Runner信号
    connect(m_pythonRunner, &PythonRunner::finished, this, &ConfigWidget::onPythonScriptFinished);
    connect(m_pythonRunner, &PythonRunner::logOutput, this, &ConfigWidget::onPythonLogOutput);

    connect(m_pythonRunner, &PythonRunner::logOutput, this, &ConfigWidget::onPythonLogOutput);

    // 绑定配置控件信号
    connect(this, &ConfigWidget::confirmConfig, this, &ConfigWidget::onConfigConfirmed);

    // 加载测试记录
    loadTestRecords();
}

ConfigWidget::~ConfigWidget() {
    delete ui;
}

void ConfigWidget::initUI() {
    // 设置按钮点击事件
    connect(ui->btnConfirm, &QPushButton::clicked, this, &ConfigWidget::on_btnConfirm_clicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &ConfigWidget::on_btnCancel_clicked);

    // ====================== 1. campaign_params 组默认值 ======================
    ui->spinInitSeed->setValue(1);
    ui->spinCampaignRun->setValue(2000);
    ui->spinTotRuns->setValue(20);
    ui->spinMaxLoopNumber->setValue(1);
    ui->spinMaxIters->setValue(5000);
    ui->lineEditSystemName->setText("nomoto_ship");
    ui->doubleSpinXStar1->setValue(30.0);
    ui->doubleSpinXStar2->setValue(0.0);

    // ====================== 2. learner_params 组默认值 ======================
    ui->spinN->setValue(500);
    ui->spinNMax->setValue(1000);
    ui->checkBoxSlidingWindow->setChecked(true);
    ui->doubleSpinLearningRate->setValue(0.01);
    ui->doubleSpinLearningRateC->setValue(0.1);
    ui->checkBoxUseScheduler->setChecked(true);
    ui->spinSchedT->setValue(300);
    ui->spinPrintInterval->setValue(200);

    // ====================== 3. lyap_params 组默认值 ======================
    ui->spinNInput->setValue(2);
    ui->doubleSpinBetaSfpl->setValue(2.0);
    ui->checkBoxClippingV->setChecked(true);
    ui->lineEditSizeLayers->setText("[10,10,1]");
    ui->lineEditLyapActivations->setText("[\"pow2\",\"linear\",\"linear\"]");
    ui->lineEditLyapBias->setText("[false,false,false]");

    // ====================== 4. control_params 组默认值 ======================
    ui->checkBoxUseLinCtr->setChecked(false);
    ui->checkBoxLinContrBias->setChecked(false);
    ui->checkBoxControlInitialised->setChecked(false);
    ui->lineEditInitControl->setText("[[-23.58639732, -5.31421063]]");
    ui->lineEditSizeCtrlLayers->setText("[50,1]");
    ui->lineEditCtrlBias->setText("[true,false]");
    ui->lineEditCtrlActivations->setText("[\"tanh\",\"linear\"]");
    ui->checkBoxUseSaturation->setChecked(true);
    ui->lineEditCtrlSat->setText("[35.0]");

    // ====================== 5. falsifier_params 组默认值 ======================
    ui->doubleSpinGammaUnderbar->setValue(0.1);
    ui->doubleSpinGammaOverbar->setValue(10.0);
    ui->spinZetaSMT->setValue(200);
    ui->doubleSpinEpsilon->setValue(0.0);
    ui->spinGridPoints->setValue(50);
    ui->spinZetaD->setValue(50);

    // ====================== 6. loss_function 组默认值 ======================
    ui->doubleSpinAlpha1->setValue(1.0);
    ui->doubleSpinAlpha2->setValue(1.0);
    ui->doubleSpinAlpha3->setValue(1.0);
    ui->doubleSpinAlpha4->setValue(0.0);
    ui->doubleSpinAlphaRoa->setValue(1.0);
    ui->doubleSpinAlpha5->setValue(1.0);

    // ====================== 7. dyn_sys_params 组默认值 ======================
    ui->spinN1->setValue(1);
    ui->spinN2->setValue(30);
    ui->doubleSpinK->setValue(0.478);
    ui->spinT->setValue(216);
    ui->spinD->setValue(0);

    // ====================== 8. postproc_params 组默认值 ======================
    ui->checkBoxExecutePostprocessing->setChecked(true);
    ui->checkBoxVerboseInfo->setChecked(true);
    ui->spinDpi->setValue(300);
    ui->checkBoxPlotV->setChecked(true);
    ui->checkBoxPlotVdot->setChecked(true);
    ui->checkBoxPlotU->setChecked(true);
    ui->checkBoxPlot4D->setChecked(true);
    ui->spinNPoints4D->setValue(500);
    ui->spinNPoints3D->setValue(100);
    ui->checkBoxPlotCtrWeights->setChecked(true);
    ui->checkBoxPlotVWeights->setChecked(true);
    ui->checkBoxPlotDataset->setChecked(true);

    // ====================== 9. closed_loop_params 组默认值 ======================
    ui->checkBoxTestClosedLoopDynamics->setChecked(true);
    ui->doubleSpinEndTime->setValue(300.0);
    ui->doubleSpinDt->setValue(0.01);
}

void ConfigWidget::bindParamsToUI() {
    // 接口保留，无实际逻辑（参数映射已在loadConfigParams/getConfigParams中完成）
}

QWidget* ConfigWidget::getTestTableWidget()
{
    QWidget *resultWidget = new QWidget(this);
    QVBoxLayout *resultMainLayout = new QVBoxLayout(resultWidget);
    resultMainLayout->setContentsMargins(10, 10, 10, 10);
    resultMainLayout->setSpacing(10);

    // 4.1 结果表格
    QGroupBox *tableGroup = new QGroupBox("测试结果列表", this);
    QVBoxLayout *tableLayout = new QVBoxLayout(tableGroup);

    QTableView *tableViewTest = new QTableView(this);
    tableViewTest->setModel(m_testTableModel);
    // 表格样式优化
    tableViewTest->setAlternatingRowColors(true);
    tableViewTest->setStyleSheet("alternate-background-color: #f0f8ff;");
    tableViewTest->horizontalHeader()->setStretchLastSection(true);
    tableViewTest->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableViewTest->verticalHeader()->setVisible(false); // 隐藏行号
    connect(tableViewTest, &QTableView::clicked, this, &ConfigWidget::onTableViewClicked);

    tableLayout->addWidget(tableViewTest);
    tableGroup->setLayout(tableLayout);

    // 4.2 刷新按钮
    QPushButton *btnRefresh = new QPushButton("刷新结果列表", this);
    connect(btnRefresh, &QPushButton::clicked, this, &ConfigWidget::loadTestRecords);

    // 组装结果保存页面布局
    resultMainLayout->addWidget(tableGroup);
    resultMainLayout->addWidget(btnRefresh, 0, Qt::AlignRight);
    resultWidget->setLayout(resultMainLayout);
    return resultWidget;
}

void ConfigWidget::loadConfigParams(const ConfigParams& params) {
    // ====================== 1. campaign_params 组 ======================
    ui->spinInitSeed->setValue(params.init_seed);
    ui->spinCampaignRun->setValue(params.campaign_run);
    ui->spinTotRuns->setValue(params.tot_runs);
    ui->spinMaxLoopNumber->setValue(params.max_loop_number);
    ui->spinMaxIters->setValue(params.max_iters);
    ui->lineEditSystemName->setText(params.system_name);
    ui->doubleSpinXStar1->setValue(params.x_star_1);
    ui->doubleSpinXStar2->setValue(params.x_star_2);

    // ====================== 2. learner_params 组 ======================
    ui->spinN->setValue(params.N);
    ui->spinNMax->setValue(params.N_max);
    ui->checkBoxSlidingWindow->setChecked(params.sliding_window);
    ui->doubleSpinLearningRate->setValue(params.learning_rate);
    ui->doubleSpinLearningRateC->setValue(params.learning_rate_c);
    ui->checkBoxUseScheduler->setChecked(params.use_scheduler);
    ui->spinSchedT->setValue(params.sched_T);
    ui->spinPrintInterval->setValue(params.print_interval);

    // ====================== 3. lyap_params 组 ======================
    ui->spinNInput->setValue(params.n_input);
    ui->doubleSpinBetaSfpl->setValue(params.beta_sfpl);
    ui->checkBoxClippingV->setChecked(params.clipping_V);
    ui->lineEditSizeLayers->setText(params.size_layers);
    ui->lineEditLyapActivations->setText(params.lyap_activations);
    ui->lineEditLyapBias->setText(params.lyap_bias);

    // ====================== 4. control_params 组 ======================
    ui->checkBoxUseLinCtr->setChecked(params.use_lin_ctr);
    ui->checkBoxLinContrBias->setChecked(params.lin_contr_bias);
    ui->checkBoxControlInitialised->setChecked(params.control_initialised);
    ui->lineEditInitControl->setText(params.init_control);
    ui->lineEditSizeCtrlLayers->setText(params.size_ctrl_layers);
    ui->lineEditCtrlBias->setText(params.ctrl_bias);
    ui->lineEditCtrlActivations->setText(params.ctrl_activations);
    ui->checkBoxUseSaturation->setChecked(params.use_saturation);
    ui->lineEditCtrlSat->setText(params.ctrl_sat);

    // ====================== 5. falsifier_params 组 ======================
    ui->doubleSpinGammaUnderbar->setValue(params.gamma_underbar);
    ui->doubleSpinGammaOverbar->setValue(params.gamma_overbar);
    ui->spinZetaSMT->setValue(params.zeta_SMT);
    ui->doubleSpinEpsilon->setValue(params.epsilon);
    ui->spinGridPoints->setValue(params.grid_points);
    ui->spinZetaD->setValue(params.zeta_D);

    // ====================== 6. loss_function 组 ======================
    ui->doubleSpinAlpha1->setValue(params.alpha_1);
    ui->doubleSpinAlpha2->setValue(params.alpha_2);
    ui->doubleSpinAlpha3->setValue(params.alpha_3);
    ui->doubleSpinAlpha4->setValue(params.alpha_4);
    ui->doubleSpinAlphaRoa->setValue(params.alpha_roa);
    ui->doubleSpinAlpha5->setValue(params.alpha_5);

    // ====================== 7. dyn_sys_params 组 ======================
    ui->spinN1->setValue(params.n1);
    ui->spinN2->setValue(params.n2);
    ui->doubleSpinK->setValue(params.K);
    ui->spinT->setValue(params.T);
    ui->spinD->setValue(params.d);

    // ====================== 8. postproc_params 组 ======================
    ui->checkBoxExecutePostprocessing->setChecked(params.execute_postprocessing);
    ui->checkBoxVerboseInfo->setChecked(params.verbose_info);
    ui->spinDpi->setValue(params.dpi_);
    ui->checkBoxPlotV->setChecked(params.plot_V);
    ui->checkBoxPlotVdot->setChecked(params.plot_Vdot);
    ui->checkBoxPlotU->setChecked(params.plot_u);
    ui->checkBoxPlot4D->setChecked(params.plot_4D_);
    ui->spinNPoints4D->setValue(params.n_points_4D);
    ui->spinNPoints3D->setValue(params.n_points_3D);
    ui->checkBoxPlotCtrWeights->setChecked(params.plot_ctr_weights);
    ui->checkBoxPlotVWeights->setChecked(params.plot_V_weights);
    ui->checkBoxPlotDataset->setChecked(params.plot_dataset);

    // ====================== 9. closed_loop_params 组 ======================
    ui->checkBoxTestClosedLoopDynamics->setChecked(params.test_closed_loop_dynamics);
    ui->doubleSpinEndTime->setValue(params.end_time);
    ui->doubleSpinDt->setValue(params.Dt);
}

ConfigParams ConfigWidget::getConfigParams() const {
    ConfigParams params;

    // ====================== 1. campaign_params 组 ======================
    params.init_seed = ui->spinInitSeed->value();
    params.campaign_run = ui->spinCampaignRun->value();
    params.tot_runs = ui->spinTotRuns->value();
    params.max_loop_number = ui->spinMaxLoopNumber->value();
    params.max_iters = ui->spinMaxIters->value();
    params.system_name = ui->lineEditSystemName->text().trimmed();
    params.x_star_1 = ui->doubleSpinXStar1->value();
    params.x_star_2 = ui->doubleSpinXStar2->value();

    // ====================== 2. learner_params 组 ======================
    params.N = ui->spinN->value();
    params.N_max = ui->spinNMax->value();
    params.sliding_window = ui->checkBoxSlidingWindow->isChecked();
    params.learning_rate = ui->doubleSpinLearningRate->value();
    params.learning_rate_c = ui->doubleSpinLearningRateC->value();
    params.use_scheduler = ui->checkBoxUseScheduler->isChecked();
    params.sched_T = ui->spinSchedT->value();
    params.print_interval = ui->spinPrintInterval->value();

    // ====================== 3. lyap_params 组 ======================
    params.n_input = ui->spinNInput->value();
    params.beta_sfpl = ui->doubleSpinBetaSfpl->value();
    params.clipping_V = ui->checkBoxClippingV->isChecked();
    params.size_layers = ui->lineEditSizeLayers->text().trimmed();
    params.lyap_activations = ui->lineEditLyapActivations->text().trimmed();
    params.lyap_bias = ui->lineEditLyapBias->text().trimmed();

    // ====================== 4. control_params 组 ======================
    params.use_lin_ctr = ui->checkBoxUseLinCtr->isChecked();
    params.lin_contr_bias = ui->checkBoxLinContrBias->isChecked();
    params.control_initialised = ui->checkBoxControlInitialised->isChecked();
    params.init_control = ui->lineEditInitControl->text().trimmed();
    params.size_ctrl_layers = ui->lineEditSizeCtrlLayers->text().trimmed();
    params.ctrl_bias = ui->lineEditCtrlBias->text().trimmed();
    params.ctrl_activations = ui->lineEditCtrlActivations->text().trimmed();
    params.use_saturation = ui->checkBoxUseSaturation->isChecked();
    params.ctrl_sat = ui->lineEditCtrlSat->text().trimmed();

    // ====================== 5. falsifier_params 组 ======================
    params.gamma_underbar = ui->doubleSpinGammaUnderbar->value();
    params.gamma_overbar = ui->doubleSpinGammaOverbar->value();
    params.zeta_SMT = ui->spinZetaSMT->value();
    params.epsilon = ui->doubleSpinEpsilon->value();
    params.grid_points = ui->spinGridPoints->value();
    params.zeta_D = ui->spinZetaD->value();

    // ====================== 6. loss_function 组 ======================
    params.alpha_1 = ui->doubleSpinAlpha1->value();
    params.alpha_2 = ui->doubleSpinAlpha2->value();
    params.alpha_3 = ui->doubleSpinAlpha3->value();
    params.alpha_4 = ui->doubleSpinAlpha4->value();
    params.alpha_roa = ui->doubleSpinAlphaRoa->value();
    params.alpha_5 = ui->doubleSpinAlpha5->value();

    // ====================== 7. dyn_sys_params 组 ======================
    params.n1 = ui->spinN1->value();
    params.n2 = ui->spinN2->value();
    params.K = ui->doubleSpinK->value();
    params.T = ui->spinT->value();
    params.d = ui->spinD->value();

    // ====================== 8. postproc_params 组 ======================
    params.execute_postprocessing = ui->checkBoxExecutePostprocessing->isChecked();
    params.verbose_info = ui->checkBoxVerboseInfo->isChecked();
    params.dpi_ = ui->spinDpi->value();
    params.plot_V = ui->checkBoxPlotV->isChecked();
    params.plot_Vdot = ui->checkBoxPlotVdot->isChecked();
    params.plot_u = ui->checkBoxPlotU->isChecked();
    params.plot_4D_ = ui->checkBoxPlot4D->isChecked();
    params.n_points_4D = ui->spinNPoints4D->value();
    params.n_points_3D = ui->spinNPoints3D->value();
    params.plot_ctr_weights = ui->checkBoxPlotCtrWeights->isChecked();
    params.plot_V_weights = ui->checkBoxPlotVWeights->isChecked();
    params.plot_dataset = ui->checkBoxPlotDataset->isChecked();

    // ====================== 9. closed_loop_params 组 ======================
    params.test_closed_loop_dynamics = ui->checkBoxTestClosedLoopDynamics->isChecked();
    params.end_time = ui->doubleSpinEndTime->value();
    params.Dt = ui->doubleSpinDt->value();

    return params;
}

void ConfigWidget::setEditLocked(bool locked) {
    m_isLocked = locked;

    // ====================== 1. campaign_params 组 ======================
    ui->spinInitSeed->setDisabled(locked);
    ui->spinCampaignRun->setDisabled(locked);
    ui->spinTotRuns->setDisabled(locked);
    ui->spinMaxLoopNumber->setDisabled(locked);
    ui->spinMaxIters->setDisabled(locked);
    ui->lineEditSystemName->setDisabled(locked);
    ui->doubleSpinXStar1->setDisabled(locked);
    ui->doubleSpinXStar2->setDisabled(locked);

    // ====================== 2. learner_params 组 ======================
    ui->spinN->setDisabled(locked);
    ui->spinNMax->setDisabled(locked);
    ui->checkBoxSlidingWindow->setDisabled(locked);
    ui->doubleSpinLearningRate->setDisabled(locked);
    ui->doubleSpinLearningRateC->setDisabled(locked);
    ui->checkBoxUseScheduler->setDisabled(locked);
    ui->spinSchedT->setDisabled(locked);
    ui->spinPrintInterval->setDisabled(locked);

    // ====================== 3. lyap_params 组 ======================
    ui->spinNInput->setDisabled(locked);
    ui->doubleSpinBetaSfpl->setDisabled(locked);
    ui->checkBoxClippingV->setDisabled(locked);
    ui->lineEditSizeLayers->setDisabled(locked);
    ui->lineEditLyapActivations->setDisabled(locked);
    ui->lineEditLyapBias->setDisabled(locked);

    // ====================== 4. control_params 组 ======================
    ui->checkBoxUseLinCtr->setDisabled(locked);
    ui->checkBoxLinContrBias->setDisabled(locked);
    ui->checkBoxControlInitialised->setDisabled(locked);
    ui->lineEditInitControl->setDisabled(locked);
    ui->lineEditSizeCtrlLayers->setDisabled(locked);
    ui->lineEditCtrlBias->setDisabled(locked);
    ui->lineEditCtrlActivations->setDisabled(locked);
    ui->checkBoxUseSaturation->setDisabled(locked);
    ui->lineEditCtrlSat->setDisabled(locked);

    // ====================== 5. falsifier_params 组 ======================
    ui->doubleSpinGammaUnderbar->setDisabled(locked);
    ui->doubleSpinGammaOverbar->setDisabled(locked);
    ui->spinZetaSMT->setDisabled(locked);
    ui->doubleSpinEpsilon->setDisabled(locked);
    ui->spinGridPoints->setDisabled(locked);
    ui->spinZetaD->setDisabled(locked);

    // ====================== 6. loss_function 组 ======================
    ui->doubleSpinAlpha1->setDisabled(locked);
    ui->doubleSpinAlpha2->setDisabled(locked);
    ui->doubleSpinAlpha3->setDisabled(locked);
    ui->doubleSpinAlpha4->setDisabled(locked);
    ui->doubleSpinAlphaRoa->setDisabled(locked);
    ui->doubleSpinAlpha5->setDisabled(locked);

    // ====================== 7. dyn_sys_params 组 ======================
    ui->spinN1->setDisabled(locked);
    ui->spinN2->setDisabled(locked);
    ui->doubleSpinK->setDisabled(locked);
    ui->spinT->setDisabled(locked);
    ui->spinD->setDisabled(locked);

    // ====================== 8. postproc_params 组 ======================
    ui->checkBoxExecutePostprocessing->setDisabled(locked);
    ui->checkBoxVerboseInfo->setDisabled(locked);
    ui->spinDpi->setDisabled(locked);
    ui->checkBoxPlotV->setDisabled(locked);
    ui->checkBoxPlotVdot->setDisabled(locked);
    ui->checkBoxPlotU->setDisabled(locked);
    ui->checkBoxPlot4D->setDisabled(locked);
    ui->spinNPoints4D->setDisabled(locked);
    ui->spinNPoints3D->setDisabled(locked);
    ui->checkBoxPlotCtrWeights->setDisabled(locked);
    ui->checkBoxPlotVWeights->setDisabled(locked);
    ui->checkBoxPlotDataset->setDisabled(locked);

    // ====================== 9. closed_loop_params 组 ======================
    ui->checkBoxTestClosedLoopDynamics->setDisabled(locked);
    ui->doubleSpinEndTime->setDisabled(locked);
    ui->doubleSpinDt->setDisabled(locked);

    // 更新按钮状态
    ui->btnConfirm->setDisabled(locked);
    ui->btnCancel->setEnabled(locked);
}

void ConfigWidget::on_btnConfirm_clicked() {
    // 获取配置参数并发送信号
//    ConfigParams params = getConfigParams();
//    emit confirmConfig(params);
    // 锁定编辑
    setEditLocked(true);
}

void ConfigWidget::on_btnCancel_clicked() {
    // 解锁编辑
    setEditLocked(false);
    emit cancelConfig();
}

// ========== 核心业务逻辑槽函数 ==========
void ConfigWidget::loadTestRecords()
{
    QList<TestRecord> records = m_testDbHelper->getAllTestRecords(m_UUID);
    m_testTableModel->loadTestRecords(records);
    ui->textEditLog->append(QString("[%1] 已加载 %2 条测试记录").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")).arg(records.size()));
}

void ConfigWidget::onConfigConfirmed(const ConfigParams& params)
{
    // 校验测试名称和代号
    m_currentTestName = ui->lineEditTestName->text().trimmed();
    m_currentTestCode = ui->lineEditTestCode->text().trimmed();

    if (m_currentTestName.isEmpty() || m_currentTestCode.isEmpty()) {
        QMessageBox::warning(this, "输入校验", "测试名称和测试代号不能为空！");
        this->setEditLocked(false); // 解锁编辑
        return;
    }

    // 保存配置参数到数据库
    int configId = -1;
//    qDebug() << m_UUID << ": I am user id.";
    if (!m_testDbHelper->saveConfigParams(params, m_UUID, configId)) {
        QMessageBox::critical(this, "数据库错误", "保存配置参数失败！");
        this->setEditLocked(false); // 解锁编辑
        return;
    }
    m_currentConfigId = configId;

    // 转换配置参数为JSON字符串
    QJsonObject paramsJson = params.toJson();
    QString paramsDetail = QJsonDocument(paramsJson).toJson(QJsonDocument::Indented);

    // 保存测试记录（基础信息）
    TestRecord record;
    record.UUID = m_UUID;
    record.test_name = m_currentTestName;
    record.test_code = m_currentTestCode;
    record.params_detail = paramsDetail;
    record.config_id = configId;

    if (!m_testDbHelper->addTestRecord(record)) {
        QMessageBox::critical(this, "数据库错误", "保存测试记录失败！");
        this->setEditLocked(false); // 解锁编辑
        return;
    }

    // 启动Python脚本
    m_pythonRunner->setScriptPath(ui->lineEditPythonScript->text().trimmed());
    m_pythonRunner->setScriptParams(paramsJson);

    ui->textEditLog->append(QString("[%1] 启动Python脚本：%2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")).arg(ui->lineEditPythonScript->text()));
    if (!m_pythonRunner->start()) {
        QMessageBox::critical(this, "脚本启动失败", "无法启动Python脚本，请检查脚本路径和环境配置！");
        this->setEditLocked(false); // 解锁编辑
    }

    // 日志提示：仅输出信息，不修改布局
    ui->textEditLog->append(QString("[%1] 配置参数已确认，启动Python脚本...").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")));
    ui->textEditLog->append(QString("测试名称：%1，测试代号：%2").arg(m_currentTestName).arg(m_currentTestCode));

    if (!m_pythonRunner->start()) {
        QMessageBox::critical(this, "脚本启动失败", "无法启动Python脚本，请检查脚本路径和环境配置！");
        this->setEditLocked(false); // 解锁编辑
    }
}

void ConfigWidget::onPythonScriptFinished(bool success, const QDateTime& execTime, const QString& metricsData)
{
    // 1. 恢复按钮状态：启用启动，禁用中断
    ui->btnStartAlgorithm->setEnabled(true);
    ui->btnInterrupt->setDisabled(true);

    // 2. 解锁配置控件
    this->setEditLocked(false);

    // 3. 更新测试记录
    QList<TestRecord> records = m_testDbHelper->getAllTestRecords(m_UUID);
    TestRecord targetRecord;
    for (const TestRecord& record : records) {
        if (record.test_name == m_currentTestName && record.test_code == m_currentTestCode && record.config_id == m_currentConfigId) {
            targetRecord = record;
            break;
        }
    }

    if (targetRecord.test_id == -1) {
        ui->textEditLog->append(QString("[%1] 错误：未找到对应的测试记录").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")));
        QMessageBox::warning(this, "提示", "未找到对应的测试记录！");
        return;
    }

    // 更新测试记录
    targetRecord.UUID = m_UUID;
    targetRecord.execute_time = execTime;
    targetRecord.metrics_data = metricsData;
    targetRecord.result_path = m_pythonRunner->getResultPath();
    targetRecord.remark = success ? "执行成功" : "执行失败";

    if (m_testDbHelper->updateTestRecord(targetRecord)) {
        loadTestRecords(); // 刷新表格
        if (success) {
            ui->textEditLog->append(QString("[%1] ✅ 算法测试执行成功（测试名称：%2，代号：%3）").arg(
                execTime.toString("yyyy-MM-dd HH:mm:ss"),
                m_currentTestName,
                m_currentTestCode
            ));
            ui->textEditLog->append(QString("[%1] 结果路径：%2").arg(
                execTime.toString("yyyy-MM-dd HH:mm:ss"),
                targetRecord.result_path
            ));
            QMessageBox::information(this, "执行成功", "Python脚本执行完成，结果已保存！");
        } else {
            ui->textEditLog->append(QString("[%1] ❌ 算法测试执行失败（测试名称：%2，代号：%3）").arg(
                execTime.toString("yyyy-MM-dd HH:mm:ss"),
                m_currentTestName,
                m_currentTestCode
            ));
            QMessageBox::critical(this, "执行失败", "Python脚本执行失败，请查看日志！");
        }
    } else {
        QMessageBox::critical(this, "数据库错误", "更新测试记录失败！");
    }
}

void ConfigWidget::onPythonLogOutput(const QString& log)
{
    // 输出日志到文本框
    // 1. 读取Python输出的字节流（原始UTF-8编码）
    QByteArray logData = log.toUtf8();
    if (logData.isEmpty()) return;

    // 2. 关键：将UTF-8字节流解码为QString（解决乱码的核心步骤）
    // Qt5中直接用QString的fromUtf8方法，无需设置文档编码
    QString logStr = QString::fromUtf8(logData);

    // 3. 格式化日志（可选，保留时间戳）
    QString timeStamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString formattedLog = QString("[%1] %2").arg(timeStamp, logStr);

    // 4. 插入到QTextEdit（自动适配UTF-16，无乱码）
    // 避免逐行插入时的闪烁，使用append而非insertPlainText
    ui->textEditLog->append(formattedLog);

    // 可选：自动滚动到最新日志
    QTextCursor cursor = ui->textEditLog->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->textEditLog->setTextCursor(cursor);
}

void ConfigWidget::onTableViewClicked(const QModelIndex& index)
{
    if (!index.isValid()) return;

    int row = index.row();
    TestRecord record = m_testTableModel->getRecordAt(row);

    switch (index.column()) {
    case TestTableModel::ColParamsDetail: // 参数详情列
        showParamsDetailDialog(record.params_detail);
        break;
    case TestTableModel::ColResultView: // 结果查看列
        if (!record.result_path.isEmpty()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(record.result_path));
        } else {
            QMessageBox::warning(this, "提示", "暂无结果文件夹路径！");
        }
        break;
    case TestTableModel::ColMetrics: // 指标分析列
        showMetricsDialog(record.metrics_data);
        break;
    case TestTableModel::ColEditDelete: // 编辑/删除列
        showEditDeleteDialog(row, record);
        break;
    default:
        break;
    }
}

void ConfigWidget::showParamsDetailDialog(const QString& paramsJson)
{
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("参数详情");
    dialog->resize(800, 600);
    dialog->setModal(true);

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

void ConfigWidget::showMetricsDialog(const QString& metricsJson)
{
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("指标分析（超调量/调节时间/稳态误差）");
    dialog->resize(600, 400);
    dialog->setModal(true);

    QTextEdit* textEdit = new QTextEdit(dialog);
    textEdit->setReadOnly(true);

    if (metricsJson.isEmpty()) {
        textEdit->setText("暂无指标数据（脚本未输出metrics.json）");
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

void ConfigWidget::showEditDeleteDialog(int row, const TestRecord& record)
{
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("编辑/删除测试记录");
    dialog->resize(400, 300);
    dialog->setModal(true);

    // 编辑控件
    QLineEdit* lineEditTestName = new QLineEdit(record.test_name, dialog);
    lineEditTestName->setPlaceholderText("测试名称");

    QLineEdit* lineEditTestCode = new QLineEdit(record.test_code, dialog);
    lineEditTestCode->setPlaceholderText("测试代号");

    QTextEdit* textEditRemark = new QTextEdit(record.remark, dialog);
    textEditRemark->setPlaceholderText("备注（如：执行结果说明）");

    // 按钮
    QPushButton* btnSave = new QPushButton("保存", dialog);
    QPushButton* btnDelete = new QPushButton("删除", dialog);
    QPushButton* btnCancel = new QPushButton("取消", dialog);

    // 保存逻辑
    connect(btnSave, &QPushButton::clicked, this, [=]() {
        TestRecord updatedRecord = record;
        updatedRecord.test_name = lineEditTestName->text().trimmed();
        updatedRecord.test_code = lineEditTestCode->text().trimmed();
        updatedRecord.remark = textEditRemark->toPlainText().trimmed();

        if (updatedRecord.test_name.isEmpty() || updatedRecord.test_code.isEmpty()) {
            QMessageBox::warning(this, "输入校验", "测试名称和代号不能为空！");
            return;
        }

        if (m_testDbHelper->updateTestRecord(updatedRecord)) {
            m_testTableModel->updateRecordAt(row, updatedRecord);
            ui->textEditLog->append(QString("[%1] 测试记录 %2 已更新").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")).arg(updatedRecord.test_code));
            QMessageBox::information(this, "成功", "测试记录更新成功！");
            dialog->close();
        } else {
            QMessageBox::critical(this, "错误", "测试记录更新失败！");
        }
    });

    // 删除逻辑
    connect(btnDelete, &QPushButton::clicked, this, [=]() {
        if (QMessageBox::question(this, "确认删除", "确定要删除该测试记录吗？\n删除后不可恢复！") == QMessageBox::Yes) {
            if (m_testDbHelper->deleteTestRecord(record.test_id)) {
                m_testTableModel->removeRecordAt(row);
                ui->textEditLog->append(QString("[%1] 测试记录 %2 已删除").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")).arg(record.test_code));
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
    vLayout->addWidget(new QLabel("测试名称：", dialog));
    vLayout->addWidget(lineEditTestName);
    vLayout->addWidget(new QLabel("测试代号：", dialog));
    vLayout->addWidget(lineEditTestCode);
    vLayout->addWidget(new QLabel("备注：", dialog));
    vLayout->addWidget(textEditRemark);

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addWidget(btnSave);
    hLayout->addWidget(btnDelete);
    hLayout->addWidget(btnCancel);

    vLayout->addLayout(hLayout);
    dialog->setLayout(vLayout);

    dialog->exec();
    dialog->deleteLater();
}

void ConfigWidget::onBtnStartAlgorithmClicked()
{
    // 1. 基础校验
    m_currentTestName = ui->lineEditTestName->text().trimmed();
    m_currentTestCode = ui->lineEditTestCode->text().trimmed();
    QString scriptPath = ui->lineEditPythonScript->text().trimmed();

    if (m_currentTestName.isEmpty() || m_currentTestCode.isEmpty()) {
        QMessageBox::warning(this, "输入校验", "测试名称和测试代号不能为空！");
        return;
    }
    if (scriptPath.isEmpty()) {
        QMessageBox::warning(this, "输入校验", "Python脚本路径不能为空！");
        return;
    }

    // 2. 从ConfigWidget获取配置参数
    ConfigParams params = this->getConfigParams();

    // 3. 锁定配置控件（防止运行中修改）
    this->setEditLocked(true);

    // 4. 更新按钮状态：禁用启动，启用中断
    ui->btnStartAlgorithm->setDisabled(true);
    ui->btnInterrupt->setEnabled(true);

    // 5. 保存配置参数到数据库
    int configId = -1;
    if (!m_testDbHelper->saveConfigParams(params, m_UUID, configId)) {
        QMessageBox::critical(this, "数据库错误", "保存配置参数失败！");
        // 恢复状态
        this->setEditLocked(false);
        ui->btnStartAlgorithm->setEnabled(true);
        ui->btnInterrupt->setDisabled(true);
        return;
    }
    m_currentConfigId = configId;

    // 6. 转换配置参数为JSON
    QJsonObject paramsJson = params.toJson();
    QString paramsDetail = QJsonDocument(paramsJson).toJson(QJsonDocument::Indented);

    // 7. 保存测试记录（基础信息）
    TestRecord record;
    record.UUID = m_UUID;
    record.test_name = m_currentTestName;
    record.test_code = m_currentTestCode;
    record.params_detail = paramsDetail;
    record.config_id = configId;
    record.execute_time = QDateTime::currentDateTime();
    record.remark = "运行中";

    if (!m_testDbHelper->addTestRecord(record)) {
        QMessageBox::critical(this, "数据库错误", "保存测试记录失败！");
        // 恢复状态
        this->setEditLocked(false);
        ui->btnStartAlgorithm->setEnabled(true);
        ui->btnInterrupt->setDisabled(true);
        return;
    }

    // 8. 启动Python脚本
    m_pythonRunner->setScriptPath(scriptPath);
    m_pythonRunner->setScriptParams(paramsJson);

    ui->textEditLog->append(QString("[%1] 启动算法测试：%2（代号：%3）").arg(
        QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"),
        m_currentTestName,
        m_currentTestCode
    ));
    ui->textEditLog->append(QString("[%1] 脚本路径：%2").arg(
        QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"),
        scriptPath
    ));

    if (!m_pythonRunner->start()) {
        QMessageBox::critical(this, "脚本启动失败", "无法启动Python脚本，请检查脚本路径和环境配置！");
        // 恢复状态
        this->setEditLocked(false);
        ui->btnStartAlgorithm->setEnabled(true);
        ui->btnInterrupt->setDisabled(true);
        // 更新测试记录备注
        record.remark = "启动失败";
        m_testDbHelper->updateTestRecord(record);
    }
}

// ========== 中断算法按钮槽函数 ==========
void ConfigWidget::onBtnInterruptClicked()
{
    // 1. 确认中断
    if (QMessageBox::question(this, "确认中断", "确定要中断当前运行的算法吗？",
        QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    // 2. 停止Python脚本
    m_pythonRunner->stop();

    // 3. 更新按钮状态：启用启动，禁用中断
    ui->btnStartAlgorithm->setEnabled(true);
    ui->btnInterrupt->setDisabled(true);

    // 4. 解锁配置控件
    this->setEditLocked(false);

    // 5. 日志提示
    ui->textEditLog->append(QString("[%1] 算法测试已手动中断（测试名称：%2，代号：%3）").arg(
        QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"),
        m_currentTestName,
        m_currentTestCode
    ));

    // 6. 更新测试记录备注
    QList<TestRecord> records = m_testDbHelper->getAllTestRecords(m_UUID);
    for (TestRecord& record : records) {
        if (record.test_name == m_currentTestName && record.test_code == m_currentTestCode && record.config_id == m_currentConfigId) {
            record.remark = "手动中断";
            m_testDbHelper->updateTestRecord(record);
            break;
        }
    }

    // 刷新测试记录列表
    loadTestRecords();
}

// ========== 恢复默认配置按钮槽函数 ==========
void ConfigWidget::onBtnResetDefaultClicked()
{
    // 1. 确认恢复默认
    if (QMessageBox::question(this, "确认恢复默认", "确定要将所有配置参数恢复为默认值吗？当前修改的参数将丢失！",
        QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    // 2. 调用ConfigWidget的initUI（重新设置默认值）
    this->initUI();

    // 3. 解锁配置控件（如果之前锁定）
    this->setEditLocked(false);

    // 4. 重置测试名称/代号输入框（可选）
    ui->lineEditTestName->clear();
    ui->lineEditTestCode->clear();
    ui->lineEditPythonScript->setText("E:\\temp\\viewplatform\\code\\clf_synthesis.py"); // 恢复脚本路径默认值

    // 5. 日志提示
    ui->textEditLog->append(QString("[%1] 所有配置参数已恢复为默认值").arg(
        QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
    ));

    QMessageBox::information(this, "操作成功", "配置参数已恢复为默认值！");
}

void ConfigWidget::onBtnSelectScriptClicked()
{
    // 1. 配置文件对话框：筛选.py文件，默认路径为当前程序目录
    QString defaultPath = QCoreApplication::applicationDirPath();
    QString selectedFile = QFileDialog::getOpenFileName(
                this,                          // 父窗口
                "选择Python脚本文件",           // 对话框标题
                defaultPath,                   // 默认打开路径
                "Python Files (*.py);;All Files (*.*)" // 文件筛选规则
                );

    // 2. 若用户选择了文件，更新输入框内容
    if (!selectedFile.isEmpty()) {
        ui->lineEditPythonScript->setText(selectedFile);
        // 日志提示（可选）
        ui->textEditLog->append(QString("[%1] 已选择Python脚本：%2").arg(
                                  QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"),
                                  selectedFile
                                  ));
    }
}
