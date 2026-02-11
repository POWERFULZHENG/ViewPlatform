#ifndef LLMWIDGET_H
#define LLMWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include "BaseDbHelper.h"
#include "ApiConfigDialog.h"

class LLMWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LLMWidget(int currentUserId, QWidget *parent = nullptr);
    ~LLMWidget() override;

private slots:
    // UI 操作槽函数
    void onToggleDialogListClicked();
    void onRenameDialogActionTriggered();
    void onDelDialogActionTriggered();
    void onDialogListContextMenuRequested(const QPoint &pos);
    void onConfigBtnClicked();
    void onNewDialogBtnClicked();
    void onDialogItemClicked(QListWidgetItem *item);
    void onSendBtnClicked();
    void onFileBtnClicked();
    void onModelChanged(int index);
    void onAddModelBtnClicked();
    void onCancelBtnClicked();

    // 网络请求相关槽函数
    void onApiReplyFinished();

private:
    // 核心初始化与数据操作函数
    void initUI();
    void loadHistoryDialogs();
    void loadModelList();
    bool saveCurrentDialog();
    bool loadDialogById(int dialogId);
    QJsonObject getApiConfig();
    bool saveSelectedModel(const QString &modelCode);

    // 网络请求函数（非流式）
    void sendApiRequest(const QString &content);
    QString extractFullContentFromResponse(const QByteArray &data);

    // 格式转换核心函数
    QString convertMarkdownToRichText(const QString &markdown);
    // 处理LaTeX公式转换为MathJax兼容格式
    QString processLatexFormulas(const QString &text);

    // 1. 核心业务数据
    int m_currentUserId = -1;
    int m_currentDialogId = -1;
    QString m_selectedModelCode;
    QString m_fileContent;

    // 2. 网络请求相关
    QNetworkAccessManager *m_netManager = nullptr;
    QNetworkReply *m_currentReply = nullptr;

    // 3. UI 状态控制
    bool m_isDialogListCollapsed = false;
    bool m_isInit = true;
    bool m_isRequestProcessing = false;

    // 4. UI 控件（菜单/动作）
    QMenu *m_dialogContextMenu = nullptr;
    QAction *m_renameAction = nullptr;
    QAction *m_delAction = nullptr;

    // 5. UI 控件（按钮-通用）
    QPushButton *m_toggleBtn = nullptr;
    QPushButton *m_sendBtn = nullptr;
    QPushButton *m_fileBtn = nullptr;
    QPushButton *m_cancelBtn = nullptr;

    // 6. UI 控件（列表/编辑框）
    QListWidget *m_dialogList = nullptr;
    QTextEdit *m_chatContentEdit = nullptr;
    QTextEdit *m_inputEdit = nullptr;
    QLabel *m_uploadedFileLabel = nullptr;

    // 7. UI 控件（模型选择）
    QComboBox *m_modelCombo = nullptr;
    QPushButton *m_addModelBtn = nullptr;
};

#endif // LLMWIDGET_H
