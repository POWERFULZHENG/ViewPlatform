#include "LLMWidget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QLabel>
#include <QInputDialog>
#include <QSqlRecord>
#include <QSqlError>
#include <QThread>
#include <QTextBlock>
#include <QMenu>
#include <QAction>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDateTime>
#include <QApplication>
#include <QTimer>
#include "loghelper.h"
#include "logmanager.h"
#include "iphelper.h"
#include <QTextCursor>
#include <QTextCharFormat>
#include <QStyle>
#include <QFileInfo>  // 新增：用于获取文件信息

// ---------- 构造与析构 ----------
LLMWidget::LLMWidget(int currentUserId, QWidget *parent)
    : QWidget(parent),  // 基类初始化（必须放在第一个）
      // ========== 严格匹配头文件声明顺序的初始化列表 ==========
      // 1. 核心业务数据
      m_currentUserId(currentUserId),
      m_currentDialogId(-1),
      m_selectedModelCode(),
      m_fileContent(),

      // 2. 网络请求相关
      m_netManager(new QNetworkAccessManager(this)),
      m_currentReply(nullptr),

      // 3. UI 状态控制
      m_isDialogListCollapsed(false),
      m_isInit(true),
      m_isRequestProcessing(false),

      // 4. UI 控件（菜单/动作）
      m_dialogContextMenu(nullptr),
      m_renameAction(nullptr),

      // 5. UI 控件（按钮-通用）
      m_toggleBtn(nullptr),
      m_sendBtn(nullptr),
      m_fileBtn(nullptr),
      m_cancelBtn(nullptr),

      // 6. UI 控件（列表/编辑框）
      m_dialogList(nullptr),
      m_chatContentEdit(nullptr),
      m_inputEdit(nullptr),
      // 文件上传显示标签
      m_uploadedFileLabel(nullptr),

      // 7. UI 控件（模型选择）
      m_modelCombo(nullptr),
      m_addModelBtn(nullptr)

{
    // 保留原有构造函数内部逻辑，无修改
    if (m_currentUserId <= 0) {
        LOG_ERROR("LLM模块", "【致命】用户ID无效：" << m_currentUserId);
        QMessageBox::critical(nullptr, "致命错误", "用户ID无效，程序无法正常初始化！");
        return;
    }

    LOG_INFO("LLM模块", "【LLMWidget】初始化，用户ID=" << m_currentUserId);
    initUI();

    QTimer::singleShot(50, this, &LLMWidget::loadModelList);
    QTimer::singleShot(120, this, &LLMWidget::loadHistoryDialogs);
}

LLMWidget::~LLMWidget()
{
    // 安全清理网络请求
    if (m_currentReply) {
        if (!m_currentReply->isFinished()) {
            m_currentReply->abort();
        }
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
}

// ---------- UI 初始化（新增MathJax支持 + 历史对话标签 + 文件上传显示） ----------
void LLMWidget::initUI()
{
    LOG_DEBUG("LLM模块", "【UI】创建控件");

    // ========== 顶部控件 ==========
    QPushButton *configBtn = new QPushButton("API配置", this);
    QPushButton *newDialogBtn = new QPushButton("新建对话", this);
    m_toggleBtn = new QPushButton("折叠列表", this);

    // 模型选择控件
    m_modelCombo = new QComboBox(this);
    m_addModelBtn = new QPushButton("+ 添加模型", this);
    QLabel *modelLabel = new QLabel("选择模型：", this);

    // 顶部布局
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(configBtn);
    topLayout->addWidget(newDialogBtn);
    topLayout->addWidget(m_toggleBtn);
    topLayout->addSpacing(20);
    topLayout->addWidget(modelLabel);
    topLayout->addWidget(m_modelCombo);
    topLayout->addWidget(m_addModelBtn);
    topLayout->addStretch();

    // ========== 左侧对话列表（修改：添加历史对话标签） ==========
    m_dialogList = new QListWidget(this);
    m_dialogList->setMinimumWidth(220);
    m_dialogList->setContextMenuPolicy(Qt::CustomContextMenu);

    // 新增：创建历史对话标签
    QLabel *dialogTitleLabel = new QLabel("历史对话", this);
    // 设置标签样式（可选，优化视觉效果）
    dialogTitleLabel->setStyleSheet("font-weight: bold; font-size: 14px; padding: 5px; color: #2c3e50;");
    dialogTitleLabel->setAlignment(Qt::AlignLeft);

    // 新增：将标签和列表封装到垂直布局
    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->addWidget(dialogTitleLabel);  // 添加历史对话标签
    leftLayout->addWidget(m_dialogList);      // 添加对话列表
    leftLayout->setContentsMargins(0, 0, 0, 0); // 去除布局边距，优化UI

    // 新增：创建左侧容器Widget，承载布局
    QWidget *leftWidget = new QWidget(this);
    leftWidget->setLayout(leftLayout);

    // 右键菜单
    m_dialogContextMenu = new QMenu(m_dialogList);
    m_renameAction = new QAction("重命名", m_dialogContextMenu);
    m_dialogContextMenu->addAction(m_renameAction);

    // ========== 右侧聊天区域（核心：公式渲染） ==========
    m_chatContentEdit = new QTextEdit(this);
    m_chatContentEdit->setReadOnly(true);
    m_chatContentEdit->setAcceptRichText(true);
    QString mathJaxHeader = R"(
        <html>
        <head>
            <meta charset="UTF-8">
            <script src="https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js"></script>
            <style>
                body { font-family: "Microsoft YaHei", Arial, sans-serif; font-size: 13px; line-height: 1.6; }
                pre { white-space: pre-wrap; word-wrap: break-word; }
                blockquote { margin: 5px 0; padding: 5px 10px; background: #f8f9fa; border-left: 3px solid #6c757d; }
                h1 { font-size: 18px; color: #2c3e50; margin: 8px 0; }
                h2 { font-size: 16px; color: #34495e; margin: 6px 0; }
                h3 { font-size: 14px; color: #415a77; margin: 5px 0; }
            </style>
        </head>
        <body>
    )";
    m_chatContentEdit->setHtml(mathJaxHeader + "</body></html>");

    // 输入区域
    m_inputEdit = new QTextEdit(this);
    m_fileBtn = new QPushButton("选择文件", this);
    m_sendBtn = new QPushButton("发送", this);
    m_cancelBtn = new QPushButton("取消", this);
    m_cancelBtn->setEnabled(false);

    // 新增：文件上传信息显示标签
    m_uploadedFileLabel = new QLabel(this);
    m_uploadedFileLabel->setStyleSheet(R"(
        QLabel {
            color: #27ae60;
            font-size: 12px;
            padding: 2px 5px;
            background-color: #f0f9f0;
            border-radius: 3px;
            border: 1px solid #d4ecd4;
        }
    )");
    m_uploadedFileLabel->setVisible(false); // 初始隐藏
    m_uploadedFileLabel->setToolTip("已上传的文件信息");

    // 底部布局（修改：添加文件显示标签）
    QHBoxLayout *bottomLayout = new QHBoxLayout();
//    bottomLayout->addWidget(m_inputEdit);
    bottomLayout->addWidget(m_fileBtn);
    bottomLayout->addWidget(m_uploadedFileLabel); // 添加文件显示标签
    bottomLayout->addWidget(m_sendBtn);
    bottomLayout->addWidget(m_cancelBtn);

    QVBoxLayout *rightLayout = new QVBoxLayout();
    m_inputEdit->setMinimumHeight(20);
    m_inputEdit->setMaximumHeight(50);
    rightLayout->addWidget(m_chatContentEdit);
    rightLayout->addWidget(m_inputEdit);
    rightLayout->addLayout(bottomLayout);

    // 主布局（修改：添加封装后的左侧Widget）
    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->addWidget(leftWidget);  // 替换原来的 m_dialogList
    mainLayout->addLayout(rightLayout, 1);

    // 根布局
    QVBoxLayout *root = new QVBoxLayout(this);
    root->addLayout(topLayout);
    root->addLayout(mainLayout);

    // ========== 信号连接 ==========
    connect(m_toggleBtn, &QPushButton::clicked, this, &LLMWidget::onToggleDialogListClicked);
    connect(m_dialogList, &QListWidget::customContextMenuRequested,
            this, &LLMWidget::onDialogListContextMenuRequested);
    connect(m_renameAction, &QAction::triggered, this, &LLMWidget::onRenameDialogActionTriggered);

    connect(configBtn, &QPushButton::clicked, this, &LLMWidget::onConfigBtnClicked);
    connect(newDialogBtn, &QPushButton::clicked, this, &LLMWidget::onNewDialogBtnClicked);
    connect(m_dialogList, &QListWidget::itemClicked, this, &LLMWidget::onDialogItemClicked);
    connect(m_sendBtn, &QPushButton::clicked, this, &LLMWidget::onSendBtnClicked);
    connect(m_fileBtn, &QPushButton::clicked, this, &LLMWidget::onFileBtnClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &LLMWidget::onCancelBtnClicked);
//    connect(m_inputEdit, &QTextEdit::returnPressed, this, &LLMWidget::onSendBtnClicked);

    connect(m_modelCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &LLMWidget::onModelChanged);
    connect(m_addModelBtn, &QPushButton::clicked, this, &LLMWidget::onAddModelBtnClicked);
}

// ---------- 历史对话加载 ----------
void LLMWidget::loadHistoryDialogs()
{
    LOG_DEBUG("LLM模块", "【DB】加载历史对话 用户ID=" << m_currentUserId);
    if (!m_dialogList) return;
    m_dialogList->clear();

    BaseDbHelper *db = BaseDbHelper::getInstance();
    if (!db || !db->checkDbConn()) {
        LOG_ERROR("LLM模块", "【DB】数据库实例为空或未连接，加载历史对话失败");
        QMessageBox::warning(this, "警告", "数据库连接失败，无法加载历史对话！");
        return;
    }

    QString sql = QString("SELECT id, dialog_title FROM chat_dialog WHERE user_id = %1 ORDER BY update_time DESC").arg(m_currentUserId);
    QSqlQuery q = db->execQuery(sql);
    if (q.lastError().isValid()) {
        LOG_ERROR("LLM模块", "加载历史对话失败：" << q.lastError().text());
        QMessageBox::warning(this, "警告", "加载历史对话失败：" + q.lastError().text());
        return;
    }

    // Add message icon to each dialog item
    QIcon msgIcon = style()->standardIcon(QStyle::SP_MessageBoxInformation);
    while (q.next()) {
        int id = q.value(0).toInt();
        QString title = q.value(1).toString();
        QListWidgetItem *it = new QListWidgetItem(title);
        it->setData(Qt::UserRole, id);
        it->setIcon(msgIcon); // Add icon
        m_dialogList->addItem(it);

        // Highlight current dialog
        if (id == m_currentDialogId) {
            m_dialogList->setCurrentItem(it);
        }
    }
    LOG_DEBUG("LLM模块", "【DB】历史对话加载完成，共" << m_dialogList->count());
}

// ---------- 模型列表加载 ----------
void LLMWidget::loadModelList()
{
    LOG_DEBUG("LLM模块", "【DB】加载模型列表");
    if (!m_modelCombo) return;
    m_modelCombo->clear();

    BaseDbHelper *db = BaseDbHelper::getInstance();
    if (!db || !db->checkDbConn()) {
        LOG_ERROR("LLM模块", "【DB】数据库实例为空或未连接，加载模型列表失败");
        m_modelCombo->addItem("智谱GLM-4.7", "glm-4.7");
        m_selectedModelCode = "glm-4.7";
        QMessageBox::warning(this, "警告", "数据库连接失败，已加载默认模型！");
        m_isInit = false;
        return;
    }

    QSqlQuery q = db->execQuery("SELECT model_code, model_name, is_default FROM model_config ORDER BY is_default DESC");
    if (q.lastError().isValid()) {
        LOG_ERROR("LLM模块", "加载模型失败：" << q.lastError().text());
        m_modelCombo->addItem("智谱GLM-4.7", "glm-4.7");
        m_selectedModelCode = "glm-4.7";
        QMessageBox::warning(this, "警告", "加载模型失败，已加载默认模型：" + q.lastError().text());
        m_isInit = false;
        return;
    }

    QString defaultCode;
    while (q.next()) {
        QString code = q.value(0).toString();
        QString name = q.value(1).toString();
        int def = q.value(2).toInt();
        m_modelCombo->addItem(name, code);
        if (def == 1) defaultCode = code;
    }

    if (m_modelCombo->count() == 0) {
        m_modelCombo->addItem("智谱GLM-4.7", "glm-4.7");
        defaultCode = "glm-4.7";
    }

    QJsonObject cfg = getApiConfig();
    QString userModel = cfg["model"].toString();
    int idx = -1;
    if (!userModel.isEmpty()) idx = m_modelCombo->findData(userModel);
    if (idx == -1 && !defaultCode.isEmpty()) idx = m_modelCombo->findData(defaultCode);
    if (idx == -1) idx = 0;

    if (idx >= 0 && idx < m_modelCombo->count()) {
        m_modelCombo->blockSignals(true);
        m_modelCombo->setCurrentIndex(idx);
        m_selectedModelCode = m_modelCombo->itemData(idx).toString();
        m_modelCombo->blockSignals(false);
    }
    m_isInit = false;
    LOG_DEBUG("LLM模块", "【DB】模型列表加载完成, 选中=" << m_selectedModelCode);
}

// ---------- 对话保存（支持富文本+公式） ----------
bool LLMWidget::saveCurrentDialog()
{
    if (!m_chatContentEdit) return false;

    QString plainText = m_chatContentEdit->toPlainText();
    QString richText = m_chatContentEdit->toHtml();
    if (plainText.isEmpty()) return false;

    // Generate dialog title (use user input preview)
    QString title = "未命名对话";

    // Construct message JSON (same as original logic)
    QJsonArray messages;
    QJsonObject systemMsg;
    systemMsg["role"] = "system";
    systemMsg["content"] = "你是一个有用的AI助手，专注于李雅普诺夫函数控制器算法的数据分析。";
    messages.append(systemMsg);

    QRegularExpression re(R"(【用户】([\s\S]*?)【AI】([\s\S]*?)(?=【用户】|$))");
    QRegularExpressionMatchIterator it = re.globalMatch(plainText);
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        QString u = m.captured(1).trimmed();
        QString a = m.captured(2).trimmed();
        if (!u.isEmpty()) {
            QJsonObject jo; jo["role"] = "user"; jo["content"] = u; messages.append(jo);
        }
        if (!a.isEmpty()) {
            QJsonObject jo; jo["role"] = "assistant"; jo["content"] = a; messages.append(jo);
        }
    }

    QJsonObject obj;
    obj["messages"] = messages;
    obj["rich_content"] = richText;
    QString json = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    BaseDbHelper *db = BaseDbHelper::getInstance();
    if (!db || !db->checkDbConn()) {
        LOG_ERROR("LLM模块", "【DB】数据库未连接，保存对话失败");
        QMessageBox::warning(this, "警告", "数据库未连接，无法保存对话！");
        return false;
    }

    if (m_currentDialogId == -1) {
        // --- Modified: Use prepared query to get last insert ID ---
        QString sql = QString("INSERT INTO chat_dialog (user_id, dialog_title, dialog_content, update_time) VALUES (%1, ?, ?, CURRENT_TIMESTAMP)").arg(m_currentUserId);
        QSqlQuery query = db->execQuery(sql); // Assume BaseDbHelper has getQuery() method; adjust if needed
        query.prepare(sql);
        query.addBindValue(title);
        query.addBindValue(json);

        if (!query.exec()) {
            LOG_ERROR("LLM模块", "Insert new dialog failed:" << query.lastError().text());
            return false;
        }

        // --- Key: Get the newly inserted dialog ID ---
        m_currentDialogId = query.lastInsertId().toInt();
        LOG_DEBUG("LLM模块", "【DB】New dialog created, ID=" << m_currentDialogId);
    } else {
        // Update existing dialog (original logic)
        QString sql = QString("UPDATE chat_dialog SET dialog_title = ?, dialog_content = ?, update_time = CURRENT_TIMESTAMP WHERE id = %1 AND user_id = %2").arg(m_currentDialogId).arg(m_currentUserId);
        QVariantList params = { title, json };
        if (!db->execPrepareSql(sql, params)) {
            LOG_ERROR("LLM模块", "Update dialog failed:" << db->getLastError());
            return false;
        }
    }
    return true;
}

// ---------- 读取对话（恢复公式格式） ----------
bool LLMWidget::loadDialogById(int dialogId)
{
    if (dialogId <= 0) return false;

    BaseDbHelper *db = BaseDbHelper::getInstance();
    if (!db || !db->checkDbConn()) {
        LOG_ERROR("LLM模块", "【DB】数据库未连接，加载对话失败");
        QMessageBox::warning(this, "警告", "数据库未连接，无法加载对话！");
        return false;
    }

    QString sql = QString("SELECT dialog_content FROM chat_dialog WHERE id = %1 AND user_id = %2").arg(dialogId).arg(m_currentUserId);
    QSqlQuery q = db->execQuery(sql);
    if (q.lastError().isValid()) {
        LOG_ERROR("LLM模块", "loadDialogById error:" << q.lastError().text());
        return false;
    }
    if (!q.next()) return false;

    QString json = q.value(0).toString();
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        LOG_ERROR("LLM模块", "对话 JSON 解析失败");
        return false;
    }

    QJsonObject root = doc.object();
    // 优先加载富文本（含公式）
    if (root.contains("rich_content") && root["rich_content"].isString()) {
        QString richText = root["rich_content"].toString();
        if (m_chatContentEdit) {
            m_chatContentEdit->setHtml(richText);
        }
    } else {
        // 兼容旧数据
        QJsonArray messages = root["messages"].toArray();
        QString plain;
        for (const QJsonValue &v : messages) {
            QJsonObject m = v.toObject();
            QString role = m["role"].toString();
            QString content = m["content"].toString();
            if (role == "user") plain += QString("【用户】%1\n").arg(content);
            else if (role == "assistant") plain += QString("【AI】%1\n").arg(content);
        }
        if (m_chatContentEdit) m_chatContentEdit->setText(plain);
    }

    m_currentDialogId = dialogId;
    return true;
}

// ---------- 读取用户 API 配置 ----------
QJsonObject LLMWidget::getApiConfig()
{
    QJsonObject cfg;
    BaseDbHelper *db = BaseDbHelper::getInstance();
    if (!db || !db->checkDbConn()) {
        LOG_ERROR("LLM模块", "【DB】数据库未连接，读取API配置失败");
        return cfg;
    }

    QString sql = QString("SELECT api_url, api_key, model, temperature FROM user_api_config WHERE user_id = %1").arg(m_currentUserId);
    QSqlQuery q = db->execQuery(sql);
    if (q.lastError().isValid()) {
        LOG_ERROR("LLM模块", "getApiConfig error:" << q.lastError().text());
        return cfg;
    }
    if (!q.next()) return cfg;
    cfg["api_url"] = q.value(0).toString();
    cfg["api_key"] = q.value(1).toString();
    cfg["model"] = q.value(2).toString();
    cfg["temperature"] = q.value(3).toDouble();
    return cfg;
}

// ---------- 保存选中模型 ----------
bool LLMWidget::saveSelectedModel(const QString &modelCode)
{
    QString trimmed = modelCode.trimmed();
    if (trimmed.isEmpty()) {
        LOG_ERROR("LLM模块", "模型编码为空");
        return false;
    }
    BaseDbHelper *db = BaseDbHelper::getInstance();
    if (!db || !db->checkDbConn()) {
        LOG_ERROR("LLM模块", "DB 未连接");
        QMessageBox::warning(this, "警告", "数据库未连接，无法保存模型！");
        return false;
    }
    QString sql = QString(R"(
        INSERT INTO user_api_config (user_id, model, api_url, api_key, temperature)
        VALUES (%1, ?, '', '', 1.0)
        ON DUPLICATE KEY UPDATE model = VALUES(model), update_time = CURRENT_TIMESTAMP
    )").arg(m_currentUserId);
    QVariantList params = { trimmed };
    bool ok = db->execPrepareSql(sql, params);
    if (!ok) LOG_ERROR("LLM模块", "保存模型失败:" << db->getLastError());
    return ok;
}

// ---------- 解析API响应内容 ----------
QString LLMWidget::extractFullContentFromResponse(const QByteArray &data)
{
    if (data.isEmpty()) return "";

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (doc.isNull() || !doc.isObject()) {
        LOG_ERROR("LLM模块", "响应JSON解析失败：" << err.errorString());
        return "";
    }

    QJsonObject root = doc.object();
    QString fullContent;

    // 适配主流LLM API（OpenAI/智谱/讯飞）
    if (root.contains("choices") && root["choices"].isArray()) {
        QJsonArray choices = root["choices"].toArray();
        if (!choices.isEmpty()) {
            QJsonObject choice = choices[0].toObject();
            if (choice.contains("message") && choice["message"].isObject()) {
                QJsonObject message = choice["message"].toObject();
                if (message.contains("content") && message["content"].isString()) {
                    fullContent = message["content"].toString();
                }
            }
        }
    }

    return fullContent;
}

// ---------- LaTeX公式处理（转换为MathJax兼容格式） ----------
QString LLMWidget::processLatexFormulas(const QString &text)
{
    if (text.isEmpty()) return "";
    QString result = text;

    // 1. 处理行内公式：$公式$ → \(公式\)（MathJax行内格式）
    result.replace(QRegularExpression("\\$(.*?)\\$"), "\\\\(\\1\\\\)");

    // 2. 处理块级公式：$$公式$$ → \[公式\]（MathJax块级格式）
    result.replace(QRegularExpression("\\$\\$(.*?)\\$\\$", QRegularExpression::DotMatchesEverythingOption), "\\\\[\\1\\\\]");

    return result;
}

// ---------- 核心：Markdown+LaTeX转富文本（修复正则转义） ----------
QString LLMWidget::convertMarkdownToRichText(const QString &markdown)
{
    if (markdown.isEmpty()) return "";

    QString html = markdown;

    // 第一步：先处理LaTeX公式（避免与Markdown语法冲突）
    html = processLatexFormulas(html);

    // 1. 处理标题：# 一级 → <h1>
    html.replace(QRegularExpression("^#{1}\\s+(.*)$", QRegularExpression::MultilineOption),
                 "<h1 style='color:#2c3e50; margin:8px 0; font-size:18px; font-weight:bold;'>\\1</h1>");
    html.replace(QRegularExpression("^#{2}\\s+(.*)$", QRegularExpression::MultilineOption),
                 "<h2 style='color:#34495e; margin:6px 0; font-size:16px; font-weight:bold;'>\\1</h2>");
    html.replace(QRegularExpression("^#{3}\\s+(.*)$", QRegularExpression::MultilineOption),
                 "<h3 style='color:#415a77; margin:5px 0; font-size:14px; font-weight:bold;'>\\1</h3>");
    html.replace(QRegularExpression("^#{4,6}\\s+(.*)$", QRegularExpression::MultilineOption),
                 "<h4 style='color:#596e8b; margin:4px 0; font-size:13px; font-weight:bold;'>\\1</h4>");

    // 2. 处理加粗：**内容** → <b>
    html.replace(QRegularExpression("\\*\\*(.*?)\\*\\*"), "<b>\\1</b>");

    // 3. 处理斜体：*内容* → <i>
    html.replace(QRegularExpression("^(?!\\*)\\*(.*?)\\*(?!\\*)"), "<i>\\1</i>");

    // 4. 处理代码块：```代码``` → <pre><code>
    html.replace(QRegularExpression("```([\\s\\S]*?)```"),
                 "<pre style='background-color:#f5f5f5; padding:8px; border-radius:4px; margin:8px 0; font-family:Consolas,Monospace; font-size:12px;'>\\1</pre>");

    // 5. 处理行内代码：`代码` → <code>
    html.replace(QRegularExpression("`(.*?)`"),
                 "<code style='background-color:#f0f0f0; padding:2px 4px; border-radius:2px; font-family:Consolas; font-size:12px;'>\\1</code>");

    // 6. 处理无序列表：-/* 开头 → <ul><li>
    html.replace(QRegularExpression("^(\\s*)-\\s+(.*)$", QRegularExpression::MultilineOption),
                 "\\1<li style='margin:2px 0; list-style:disc;'>\\2</li>");
    html.replace(QRegularExpression("^(\\s*)\\*\\s+(.*)$", QRegularExpression::MultilineOption),
                 "\\1<li style='margin:2px 0; list-style:disc;'>\\2</li>");
    // 包裹列表项
    html.replace(QRegularExpression("(<li>.*?</li>)+", QRegularExpression::DotMatchesEverythingOption),
                 "<ul style='margin:6px 0; padding-left:25px;'>\\1</ul>");

    // 7. 处理分割线：--- → <hr>
    html.replace(QRegularExpression("^---$", QRegularExpression::MultilineOption),
                 "<hr style='border:0; border-top:1px solid #eee; margin:10px 0;'>");

    // 8. 处理引用：> 内容 → <blockquote>
    html.replace(QRegularExpression("^>\\s+(.*)$", QRegularExpression::MultilineOption),
                 "<blockquote style='margin:6px 0; padding:6px 10px; background-color:#f8f9fa; border-left:3px solid #6c757d;'>\\1</blockquote>");

    // 9. 处理换行：保留换行符
    html.replace("\n", "<br/>");

    // 10. 处理链接：[文本](链接) → <a>
    html.replace(QRegularExpression("\\[([^\\]]+)\\]\\(([^)]+)\\)"),
                 "<a href='\\2' style='color:#3498db; text-decoration:none;'>\\1</a>");

    return html;
}

// ---------- API响应处理（核心：公式+格式渲染） ----------
void LLMWidget::onApiReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        reply = m_currentReply;
    }
    if (!reply) return;

    // 打印响应日志
    QByteArray responseData = reply->readAll();
    LOG_DEBUG("LLM模块", "【API响应】状态码：" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
    LOG_DEBUG("LLM模块", "【API响应】原始数据：" << QString::fromUtf8(responseData));
    reply->seek(0);

    // 恢复UI状态
    auto restoreUI = [this]() {
        m_isRequestProcessing = false;
        if (m_sendBtn) m_sendBtn->setEnabled(true);
        if (m_fileBtn) m_fileBtn->setEnabled(true);
        if (m_inputEdit) m_inputEdit->setEnabled(true);
        if (m_cancelBtn) m_cancelBtn->setEnabled(false);
    };

    // 处理错误
    if (reply->error() != QNetworkReply::NoError) {
        if (reply->error() == QNetworkReply::OperationCanceledError) {
            LOG_DEBUG("LLM模块", "用户已取消请求");
        } else {
            QString err = QString("<span style='color:#e74c3c;'>【系统】API请求失败：%1</span><br/>").arg(reply->errorString());
            if (m_chatContentEdit) {
                QMetaObject::invokeMethod(this, [this, err]() {
                    m_chatContentEdit->append(err);
                }, Qt::QueuedConnection);
            }
            LOG_ERROR("LLM模块", "API 请求错误：" << reply->errorString());
        }
        restoreUI();
        reply->deleteLater();
        if (reply == m_currentReply) m_currentReply = nullptr;
        return;
    }

    // 解析AI内容
    QString aiContent = extractFullContentFromResponse(responseData);
    LOG_DEBUG("LLM模块", "【API响应】解析后的AI内容：" << aiContent);

    // UI线程更新（核心：公式渲染）
    QMetaObject::invokeMethod(this, [this, aiContent]() {
        if (!m_chatContentEdit) return;

        QTextDocument *doc = m_chatContentEdit->document();
        if (!doc || doc->isEmpty()) {
            if (!aiContent.isEmpty()) {
                // 转换Markdown+LaTeX为富文本
                QString richAiContent = convertMarkdownToRichText(aiContent);
                m_chatContentEdit->append(QString("<span style='color:#27ae60; font-weight:bold;'>【AI】</span>%1<br/>").arg(richAiContent));
            } else {
                m_chatContentEdit->append("<span style='color:#e74c3c;'>【系统】未获取到AI回复内容（解析为空）</span><br/>");
            }
            saveCurrentDialog();
            loadHistoryDialogs();
            return;
        }

        // 移除加载占位符
        QVector<int> toRem;
        for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
            if (b.text().contains("【AI】正在处理中...")) {
                toRem.append(b.blockNumber());
            }
        }
        for (int i = toRem.size()-1; i>=0; --i) {
            QTextBlock b = doc->findBlockByNumber(toRem[i]);
            if (b.isValid()) {
                QTextCursor c(b);
                c.select(QTextCursor::BlockUnderCursor);
                c.removeSelectedText();
            }
        }

        // 追加AI回复（含公式）
        if (!aiContent.isEmpty()) {
            QString richAiContent = convertMarkdownToRichText(aiContent);
            m_chatContentEdit->append(QString("<span style='color:#27ae60; font-weight:bold;'>【AI】</span>%1<br/>").arg(richAiContent));
        } else {
            m_chatContentEdit->append("<span style='color:#e74c3c;'>【系统】未获取到AI回复内容（解析为空）</span><br/>");
        }

        saveCurrentDialog();
        loadHistoryDialogs();
    }, Qt::QueuedConnection);

    // 清理资源
    restoreUI();
    reply->deleteLater();
    if (reply == m_currentReply) m_currentReply = nullptr;
}

// ---------- 发送API请求 ----------
void LLMWidget::sendApiRequest(const QString &content)
{
    if (content.trimmed().isEmpty()) {
        QMessageBox::warning(this, "警告", "输入内容不能为空！");
        return;
    }

    if (m_isRequestProcessing || m_currentReply != nullptr) {
        QMessageBox::information(this, "提示", "当前有请求正在处理中，请等待完成或取消后再发送。");
        return;
    }

    QJsonObject cfg = getApiConfig();
    if (cfg.isEmpty() || cfg["api_url"].toString().isEmpty() || cfg["api_key"].toString().isEmpty()) {
        QMessageBox::warning(this, "警告", "请先配置 API 地址和密钥！");
        return;
    }

    // Lock UI (original logic)
    m_isRequestProcessing = true;
    if (m_sendBtn) m_sendBtn->setEnabled(false);
    if (m_fileBtn) m_fileBtn->setEnabled(false);
    if (m_inputEdit) m_inputEdit->setEnabled(false);
    if (m_cancelBtn) m_cancelBtn->setEnabled(true);

    // Construct request JSON (original logic, no changes)
    QJsonObject reqObj;
    reqObj["model"] = cfg["model"].toString();
    reqObj["temperature"] = cfg["temperature"].toDouble();
    reqObj["stream"] = false;

    QJsonArray messages;
    QJsonObject systemMsg;
    systemMsg["role"] = "system";
    systemMsg["content"] = "你是一个有用的AI助手，专注于李雅普诺夫函数控制器算法的数据分析。";
    messages.append(systemMsg);

    if (m_currentDialogId != -1 && m_chatContentEdit && !m_chatContentEdit->toPlainText().isEmpty()) {
        QRegularExpression re(R"(【用户】([\s\S]*?)【AI】([\s\S]*?)(?=【用户】|$))");
        QRegularExpressionMatchIterator it = re.globalMatch(m_chatContentEdit->toPlainText());
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            QString uc = match.captured(1).trimmed();
            QString ac = match.captured(2).trimmed();
            if (!uc.isEmpty()) { QJsonObject u; u["role"]="user"; u["content"]=uc; messages.append(u); }
            if (!ac.isEmpty()) { QJsonObject a; a["role"]="assistant"; a["content"]=ac; messages.append(a); }
        }
    }

    QJsonObject newUser; newUser["role"]="user"; newUser["content"]=content;
    messages.append(newUser);
    reqObj["messages"] = messages;

    // Construct network request (original logic, no changes)
    QNetworkRequest req(QUrl(cfg["api_url"].toString()));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", QString("Bearer %1").arg(cfg["api_key"].toString()).toUtf8());

    QByteArray postData = QJsonDocument(reqObj).toJson(QJsonDocument::Compact);
    m_currentReply = m_netManager->post(req, postData);
    connect(m_currentReply, &QNetworkReply::finished, this, &LLMWidget::onApiReplyFinished, Qt::QueuedConnection);

    // --- Step 1: Append user message to chat area ---
    if (m_chatContentEdit) {
        m_chatContentEdit->append(QString("<span style='color:#3498db; font-weight:bold;'>【用户】</span>%1<br/>").arg(content));
        m_chatContentEdit->append("<span style='color:#95a5a6;'>【AI】正在处理中...</span><br/>");
        m_chatContentEdit->moveCursor(QTextCursor::End);
    }

    // --- Step 2: Key Modification - Create dialog record immediately for new dialogs ---
    bool isNewDialog = (m_currentDialogId == -1);
    if (isNewDialog && m_chatContentEdit) {
        if (saveCurrentDialog()) {
            loadHistoryDialogs(); // Refresh history list to show new dialog
            LOG_DEBUG("LLM模块", "【UI】New dialog record created immediately after sending request");
        }
    }

    ADD_BASE_LOG("LLM模块",
                 QString("请求内容：[%1]").arg(content),
                 m_currentUserId,
                 "llm",
                 IPHelper::getLocalIP());
}

// ---------- 取消请求槽函数 ----------
void LLMWidget::onCancelBtnClicked()
{
    if (!m_currentReply) {
        QMessageBox::information(this, "提示", "当前没有进行中的请求可取消。");
        return;
    }

    QMessageBox::StandardButton rb = QMessageBox::question(this, "取消请求", "确认取消当前 AI 请求？", QMessageBox::Yes | QMessageBox::No);
    if (rb != QMessageBox::Yes) return;

    if (m_cancelBtn) m_cancelBtn->setEnabled(false);
    m_currentReply->abort();
}

// ---------- 发送按钮槽函数（修改：清空文件显示） ----------
void LLMWidget::onSendBtnClicked()
{
    if (m_isRequestProcessing || m_currentReply != nullptr) {
        QMessageBox::information(this, "提示", "当前有请求正在处理中，请等待或取消后再发送。");
        return;
    }
    QString content = m_inputEdit ? m_inputEdit->toPlainText().trimmed() : QString();
    if (content.isEmpty()) {
        QMessageBox::warning(this, "警告", "输入内容不能为空！");
        return;
    }
    if(!m_fileContent.isEmpty()) content = "需求：" + content + ". 上传的内容为：" + m_fileContent;
    sendApiRequest(content);
    m_fileContent.clear();
    m_inputEdit->clear(); // 发送后清空输入框

    // 新增：清空并隐藏文件显示标签
    if (m_uploadedFileLabel) {
        m_uploadedFileLabel->clear();
        m_uploadedFileLabel->setVisible(false);
    }
}

// ---------- 文件上传槽函数（修改：添加UI显示逻辑） ----------
void LLMWidget::onFileBtnClicked()
{
    if (m_isRequestProcessing || m_currentReply != nullptr) {
        QMessageBox::information(this, "提示", "当前有请求正在处理中，请等待或取消后再上传文件。");
        return;
    }

    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", "", "文本文件 (*.txt);;所有文件 (*.*)");
    if (filePath.isEmpty()) return;

    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "文件打开失败！");
        return;
    }
    m_fileContent = QString::fromUtf8(f.readAll());
    f.close();

    // 新增：获取文件信息并更新UI显示
    QFileInfo fileInfo(filePath);
    QString fileSizeStr;
    qint64 fileSize = fileInfo.size();
    // 格式化文件大小（B/KB/MB）
    if (fileSize < 1024) {
        fileSizeStr = QString("%1 B").arg(fileSize);
    } else if (fileSize < 1024 * 1024) {
        fileSizeStr = QString("%1 KB").arg(QString::number(fileSize / 1024.0, 'f', 1));
    } else {
        fileSizeStr = QString("%1 MB").arg(QString::number(fileSize / (1024.0 * 1024), 'f', 2));
    }

    // 设置文件显示标签内容
    if (m_uploadedFileLabel) {
        m_uploadedFileLabel->setText(QString("已上传：%1 (%2)").arg(fileInfo.fileName()).arg(fileSizeStr));
        m_uploadedFileLabel->setVisible(true);
    }

    // 提示用户文件上传成功
    QMessageBox::information(this, "成功", QString("文件「%1」上传成功！").arg(fileInfo.fileName()));
}

// ---------- 模型切换槽函数 ----------
void LLMWidget::onModelChanged(int index)
{
    if (!m_modelCombo) return;
    if (index < 0 || index >= m_modelCombo->count()) return;
    QString code = m_modelCombo->itemData(index).toString();
    if (code.isEmpty()) {
        if (!m_isInit) QMessageBox::warning(this, "警告", "选中的模型编码为空！");
        return;
    }
    bool ok = saveSelectedModel(code);
    if (!m_isInit && ok && code != m_selectedModelCode) {
        m_selectedModelCode = code;
        QMessageBox::information(this, "成功", QString("模型切换为：%1").arg(m_modelCombo->currentText()));
    } else if (!m_isInit && !ok) {
        QMessageBox::critical(this, "错误", "模型保存失败，请检查数据库配置。");
    }
}

// ---------- 添加模型槽函数 ----------
void LLMWidget::onAddModelBtnClicked()
{
    bool ok1, ok2;
    QString name = QInputDialog::getText(this, "添加模型", "请输入模型名称：", QLineEdit::Normal, "", &ok1);
    if (!ok1 || name.isEmpty()) return;
    QString code = QInputDialog::getText(this, "添加模型", "请输入模型编码：", QLineEdit::Normal, "", &ok2);
    if (!ok2 || code.isEmpty()) return;

    BaseDbHelper *db = BaseDbHelper::getInstance();
    if (!db || !db->checkDbConn()) {
        QMessageBox::critical(this, "错误", "数据库未连接，无法添加模型！");
        return;
    }

    QString sql = R"(INSERT INTO model_config (model_code, model_name, is_default) VALUES (?, ?, 0)
                     ON DUPLICATE KEY UPDATE model_name = VALUES(model_name))";
    QVariantList params = { code, name };
    if (!db->execPrepareSql(sql, params)) {
        QMessageBox::critical(this, "错误", "添加模型失败：" + db->getLastError());
        return;
    }
    QMessageBox::information(this, "成功", "模型添加成功");
    loadModelList();
}

// ---------- 新建对话槽函数（修改：清空文件显示） ----------
void LLMWidget::onNewDialogBtnClicked()
{
    if (m_chatContentEdit && !m_chatContentEdit->toPlainText().isEmpty()) {
        saveCurrentDialog();
    }
    m_currentDialogId = -1;
    if (m_chatContentEdit) m_chatContentEdit->clear();
    if (m_inputEdit) m_inputEdit->clear();

    // 新增：清空文件内容和显示
    m_fileContent.clear();
    if (m_uploadedFileLabel) {
        m_uploadedFileLabel->clear();
        m_uploadedFileLabel->setVisible(false);
    }

    loadHistoryDialogs();
}

// ---------- 对话项点击槽函数（修改：清空文件显示） ----------
void LLMWidget::onDialogItemClicked(QListWidgetItem *item)
{
    if (!item) return;
    int id = item->data(Qt::UserRole).toInt();
    loadDialogById(id);

    // 新增：切换对话时清空文件内容和显示
    m_fileContent.clear();
    if (m_uploadedFileLabel) {
        m_uploadedFileLabel->clear();
        m_uploadedFileLabel->setVisible(false);
    }
}

// ---------- 折叠/展开列表槽函数 ----------
void LLMWidget::onToggleDialogListClicked()
{
    m_isDialogListCollapsed = !m_isDialogListCollapsed;
    if (m_isDialogListCollapsed) {
        m_toggleBtn->setText("展开列表");
        if (m_dialogList) m_dialogList->hide();
    } else {
        m_toggleBtn->setText("折叠列表");
        if (m_dialogList) m_dialogList->show();
    }
    if (layout()) layout()->update();
}

// ---------- 对话列表右键菜单槽函数 ----------
void LLMWidget::onDialogListContextMenuRequested(const QPoint &pos)
{
    if (!m_dialogList) return;
    QListWidgetItem *item = m_dialogList->itemAt(pos);
    if (!item) return;
    m_dialogList->setCurrentItem(item);
    if (m_dialogContextMenu) m_dialogContextMenu->exec(m_dialogList->mapToGlobal(pos));
}

// ---------- 重命名对话槽函数 ----------
void LLMWidget::onRenameDialogActionTriggered()
{
    if (!m_dialogList) return;
    QListWidgetItem *cur = m_dialogList->currentItem();
    if (!cur) return;
    int dialogId = cur->data(Qt::UserRole).toInt();
    QString old = cur->text();
    bool ok;
    QString newTitle = QInputDialog::getText(this, "重命名对话", "新的对话标题：", QLineEdit::Normal, old, &ok);
    if (!ok || newTitle.isEmpty() || newTitle == old) return;

    BaseDbHelper *db = BaseDbHelper::getInstance();
    if (!db || !db->checkDbConn()) {
        QMessageBox::critical(this, "错误", "数据库未连接，无法重命名对话！");
        return;
    }

    QString sql = QString("UPDATE chat_dialog SET dialog_title = ?, update_time = CURRENT_TIMESTAMP WHERE id = %1 AND user_id = %2").arg(dialogId).arg(m_currentUserId);
    QVariantList params = { newTitle };
    if (db->execPrepareSql(sql, params)) {
        cur->setText(newTitle);
        QMessageBox::information(this, "成功", "重命名成功");
        loadHistoryDialogs();
    } else {
        QMessageBox::critical(this, "错误", "重命名失败：" + db->getLastError());
    }
}

// ---------- API配置槽函数 ----------
void LLMWidget::onConfigBtnClicked()
{
    ApiConfigDialog dlg(m_currentUserId, this);
    if (dlg.exec() == QDialog::Accepted) {
        loadModelList();
    }
}
