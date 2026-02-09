#include "ApiConfigDialog.h"
#include <QMessageBox>
#include <QSqlQuery>

ApiConfigDialog::ApiConfigDialog(int currentUserId, QWidget *parent)
    : QDialog(parent)
    , m_currentUserId(currentUserId)
{
    // 窗口配置
    this->setWindowTitle("API配置（多用户独立）");
    this->setFixedSize(450, 220);

    // 控件初始化
    m_apiUrlEdit = new QLineEdit(this);
    m_apiKeyEdit = new QLineEdit(this);
    m_apiKeyEdit->setEchoMode(QLineEdit::Password); // 密文显示
    m_modelEdit = new QLineEdit(this);
    m_modelEdit->setPlaceholderText("例如：glm-4.7");
    m_temperatureEdit = new QLineEdit(this);
    m_temperatureEdit->setPlaceholderText("例如：1.0");

    m_saveBtn = new QPushButton("保存", this);
    m_cancelBtn = new QPushButton("取消", this);

    // 布局
    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("API地址：", m_apiUrlEdit);
    formLayout->addRow("API密钥：", m_apiKeyEdit);
    formLayout->addRow("模型版本：", m_modelEdit);
    formLayout->addRow("温度参数：", m_temperatureEdit);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(m_saveBtn);
    btnLayout->addWidget(m_cancelBtn);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(btnLayout);
    this->setLayout(mainLayout);

    // 信号槽绑定
    connect(m_saveBtn, &QPushButton::clicked, this, &ApiConfigDialog::saveConfig);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    // 加载当前用户的配置
    loadConfig();
}

void ApiConfigDialog::loadConfig()
{
    BaseDbHelper *dbHelper = BaseDbHelper::getInstance();
    // 查询当前用户的API配置
    QString sql = QString("SELECT api_url, api_key, model, temperature FROM user_api_config WHERE user_id = %1").arg(m_currentUserId);
    QSqlQuery query = dbHelper->execQuery(sql);
    if (query.next()) {
        m_apiUrlEdit->setText(query.value(0).toString());
        m_apiKeyEdit->setText(query.value(1).toString());
        m_modelEdit->setText(query.value(2).toString());
        m_temperatureEdit->setText(query.value(3).toString());
    } else {
        // 默认值（智谱AI官方地址）
        m_apiUrlEdit->setText("https://open.bigmodel.cn/api/paas/v4/chat/completions");
        m_modelEdit->setText("glm-4.7");
        m_temperatureEdit->setText("1.0");
    }
}

void ApiConfigDialog::saveConfig()
{
    QString apiUrl = m_apiUrlEdit->text().trimmed();
    QString apiKey = m_apiKeyEdit->text().trimmed();
    QString model = m_modelEdit->text().trimmed();
    float temperature = m_temperatureEdit->text().toFloat();

    if (apiUrl.isEmpty() || apiKey.isEmpty() || model.isEmpty()) {
        QMessageBox::warning(this, "警告", "API地址、密钥、模型版本不能为空！");
        return;
    }

    BaseDbHelper *dbHelper = BaseDbHelper::getInstance();
    // 先查询是否存在配置（存在则更新，不存在则插入）
    QString checkSql = QString("SELECT id FROM user_api_config WHERE user_id = %1").arg(m_currentUserId);
    QSqlQuery checkQuery = dbHelper->execQuery(checkSql);

    if (checkQuery.next()) {
        // 更新配置
        QString updateSql = QString(R"(
            UPDATE user_api_config
            SET api_url = ?, api_key = ?, model = ?, temperature = ?
            WHERE user_id = %1
        )").arg(m_currentUserId);
        QVariantList params = {apiUrl, apiKey, model, temperature};
        if (dbHelper->execPrepareSql(updateSql, params)) {
            QMessageBox::information(this, "成功", "API配置更新成功！");
            this->accept();
        } else {
            QMessageBox::critical(this, "错误", "配置更新失败：" + dbHelper->getLastError());
        }
    } else {
        // 插入新配置
        QString insertSql = QString(R"(
            INSERT INTO user_api_config (user_id, api_url, api_key, model, temperature)
            VALUES (%1, ?, ?, ?, ?)
        )").arg(m_currentUserId);
        QVariantList params = {apiUrl, apiKey, model, temperature};
        if (dbHelper->execPrepareSql(insertSql, params)) {
            QMessageBox::information(this, "成功", "API配置保存成功！");
            this->accept();
        } else {
            QMessageBox::critical(this, "错误", "配置保存失败：" + dbHelper->getLastError());
        }
    }
}
