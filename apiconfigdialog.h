#ifndef APICONFIGDIALOG_H
#define APICONFIGDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include "BaseDbHelper.h"

class ApiConfigDialog : public QDialog
{
    Q_OBJECT
public:
    // 新增currentUserId参数，实现多用户配置隔离
    explicit ApiConfigDialog(int currentUserId, QWidget *parent = nullptr);
    ~ApiConfigDialog() = default;

private slots:
    void saveConfig();   // 保存到数据库
    void loadConfig();   // 从数据库加载

private:
    int m_currentUserId;          // 当前用户ID
    QLineEdit *m_apiUrlEdit;      // API地址
    QLineEdit *m_apiKeyEdit;      // API密钥（密文）
    QLineEdit *m_modelEdit;       // 模型版本（如glm-4.7）
    QLineEdit *m_temperatureEdit; // 温度参数
    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;
};

#endif // APICONFIGDIALOG_H
