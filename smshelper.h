#ifndef SMSHELPER_H
#define SMSHELPER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QCryptographicHash>
#include <QUrlQuery>
#include <QRandomGenerator>
#include <QFile>
#include <QDir>
#include <QMessageAuthenticationCode>
#include <QMap>

class SmsHelper : public QObject
{
    Q_OBJECT
public:
    // 重载构造函数：
    // 1. 默认构造（读取配置文件）
    explicit SmsHelper(QObject *parent = nullptr);
    // 2. 手动传入配置（灵活适配不同场景）
    SmsHelper(const QString& accessKeyId,
              const QString& accessKeySecret,
              const QString& signName,
              const QString& templateCode,
              QObject *parent = nullptr);

    // 发送验证码短信（outCode：传出本次发送的验证码，用于本地校验）
    void sendVerificationCode(const QString& phone, QString& outCode);

    // 读取JSON配置文件（可外部调用重新加载配置）
    bool loadConfigFromFile(const QString& configPath = "");

signals:
    // 发送结果信号（success:是否成功，msg:提示信息）
    void sendResult(bool success, const QString& msg);

private slots:
    // 处理API返回结果
    void slot_replyFinished(QNetworkReply* reply);

private:
    // 生成6位随机验证码
    QString generateCode();

    // 阿里云短信API签名生成（严格按阿里云规范实现）
    QString generateSignature(const QMap<QString, QString>& params);

    // 核心配置（从配置文件/构造函数传入，不再硬编码）
    QString m_accessKeyId;
    QString m_accessKeySecret;
    QString m_signName;
    QString m_templateCode;

    // 网络请求对象
    QNetworkAccessManager *m_networkManager;
    // 存储当前发送的验证码（用于后续校验）
    QString m_currentCode;
    // 默认配置文件路径
    const QString DEFAULT_CONFIG_PATH = QDir::currentPath() + "/config/sms_config.json";
};

#endif // SMSHELPER_H
