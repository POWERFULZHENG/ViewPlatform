#include "SmsHelper.h"
#include "loghelper.h"
#include <QDebug>
#include <QUrl>
#include <QTimer>
#include <QTextCodec>

// 默认构造函数：读取配置文件
SmsHelper::SmsHelper(QObject *parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &SmsHelper::slot_replyFinished);

    // 自动读取默认配置文件
    if (!loadConfigFromFile()) {
        qCritical() << "【短信配置】默认配置文件读取失败！路径：" << DEFAULT_CONFIG_PATH;
        qCritical() << "请检查 config 文件夹下是否存在 sms_config.json，且配置正确";
    }
}

// 手动传入配置的构造函数
SmsHelper::SmsHelper(const QString& accessKeyId,
                     const QString& accessKeySecret,
                     const QString& signName,
                     const QString& templateCode,
                     QObject *parent)
    : QObject(parent)
    , m_accessKeyId(accessKeyId)
    , m_accessKeySecret(accessKeySecret)
    , m_signName(signName)
    , m_templateCode(templateCode)
{
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &SmsHelper::slot_replyFinished);
}

// 读取JSON配置文件
bool SmsHelper::loadConfigFromFile(const QString& configPath)
{
    // 优先使用传入的路径，否则用默认路径
    QString filePath = configPath.isEmpty() ? DEFAULT_CONFIG_PATH : configPath;

    // 检查文件是否存在
    QFile configFile(filePath);
    if (!configFile.exists()) {
        qCritical() << "【短信配置】文件不存在：" << filePath;
        return false;
    }

    // 打开并读取文件
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "【短信配置】文件打开失败：" << configFile.errorString();
        return false;
    }

    // 解析JSON
    QByteArray jsonData = configFile.readAll();
    configFile.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qCritical() << "【短信配置】JSON解析失败：" << parseError.errorString();
        return false;
    }

    // 提取配置项
    QJsonObject rootObj = doc.object();
    QJsonObject smsObj = rootObj["aliyun_sms"].toObject();

    m_accessKeyId = smsObj["access_key_id"].toString();
    m_accessKeySecret = smsObj["access_key_secret"].toString();
    m_signName = smsObj["sign_name"].toString();
    m_templateCode = smsObj["template_code"].toString();

    // 校验配置完整性
    if (m_accessKeyId.isEmpty() || m_accessKeySecret.isEmpty() ||
        m_signName.isEmpty() || m_templateCode.isEmpty()) {
        qCritical() << "【短信配置】配置项不完整！请检查 access_key_id/sign_name 等字段";
        return false;
    }

    LOG_DEBUG("登录模块", QString("[短信配置]加载成功！签名：%1，模板：%2").arg(m_signName).arg(m_templateCode));
    return true;
}

// 生成6位随机验证码
QString SmsHelper::generateCode()
{
    return QString::number(QRandomGenerator::global()->bounded(100000, 999999));
}

// 阿里云签名生成（严格符合阿里云HMAC-SHA1规范）
QString SmsHelper::generateSignature(const QMap<QString, QString>& params)
{
    // 1. 按参数名ASCII升序排序
    QMap<QString, QString> sortedParams = params;
    QList<QString> keys = sortedParams.keys();
    std::sort(keys.begin(), keys.end());

    // 2. 拼接为 key=value&key=value 格式（value需URL编码）
    QString canonicalizedQueryString;
    for (const QString& key : keys) {
        canonicalizedQueryString += key + "=" + QUrl::toPercentEncoding(sortedParams[key]) + "&";
    }
    canonicalizedQueryString.chop(1); // 移除最后一个&

    // 3. 构造待签名字符串（固定格式：HTTPMethod&%2F&URL编码后的参数串）
    QString stringToSign = "POST&%2F&" + QUrl::toPercentEncoding(canonicalizedQueryString);

    // 4. HMAC-SHA1加密 + Base64编码（阿里云签名核心）
    QByteArray keyBytes = (m_accessKeySecret + "&").toUtf8(); // 阿里云要求末尾加&
    QMessageAuthenticationCode mac(QCryptographicHash::Sha1);
    mac.setKey(keyBytes);
    mac.addData(stringToSign.toUtf8());
    QByteArray hmacBytes = mac.result();
    QString signature = hmacBytes.toBase64();

    return signature;
}

// 发送验证码短信
void SmsHelper::sendVerificationCode(const QString& phone, QString& outCode)
{
    // 前置校验：配置是否有效
    if (m_accessKeyId.isEmpty() || m_accessKeySecret.isEmpty()) {
        emit sendResult(false, "短信配置未加载！请检查 config/sms_config.json 文件");
        return;
    }

    // 1. 生成并存储验证码
    m_currentCode = generateCode();
    outCode = m_currentCode; // 传出验证码（供本地校验）

    // 2. 构造阿里云短信API参数
    QMap<QString, QString> params;
    params["AccessKeyId"] = m_accessKeyId;
    params["Action"] = "SendSms";
    params["Format"] = "JSON"; // 返回格式
    params["PhoneNumbers"] = phone; // 目标手机号
    params["SignName"] = m_signName; // 短信签名
    params["TemplateCode"] = m_templateCode; // 模板CODE
    params["TemplateParam"] = QString("{\"code\":\"%1\"}").arg(m_currentCode); // 模板参数
    params["SignatureMethod"] = "HMAC-SHA1"; // 签名算法
    params["SignatureNonce"] = QString::number(QRandomGenerator::global()->bounded(10000000, 99999999)); // 随机数（防重放）
    params["SignatureVersion"] = "1.0"; // 签名版本
    params["Timestamp"] = QDateTime::currentDateTime().toUTC().toString("yyyy-MM-ddTHH:mm:ssZ"); // UTC时间（必须）
    params["Version"] = "2017-05-25"; // API版本

    // 3. 生成签名并添加到参数
    QString signature = generateSignature(params);
    params["Signature"] = signature;

    // 4. 构造HTTP请求（移除setTransferTimeout，改用定时器超时）
    QUrl url("https://dysmsapi.aliyuncs.com/");
    QNetworkRequest request(url);
    // 设置请求头（固定为form表单）
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=utf-8");

    // 5. 拼接POST参数（URL编码）
    QUrlQuery postData;
    for (auto it = params.begin(); it != params.end(); ++it) {
        postData.addQueryItem(it.key(), it.value());
    }
    QByteArray postBytes = postData.toString(QUrl::FullyEncoded).toUtf8();

    // 6. 发送请求 + 新增：设置10秒超时定时器
    QNetworkReply* reply = m_networkManager->post(request, postBytes);
    QTimer* timeoutTimer = new QTimer(this);
    timeoutTimer->setSingleShot(true);
    timeoutTimer->setInterval(10000); // 10秒超时

    // 绑定超时逻辑
    connect(timeoutTimer, &QTimer::timeout, this, [=]() {
        if (reply->isRunning()) {
            reply->abort(); // 终止请求
            emit sendResult(false, "短信发送超时！请检查网络或稍后重试");
            timeoutTimer->deleteLater();
        }
    });
    // 请求完成后销毁定时器
    connect(reply, &QNetworkReply::finished, this, [=]() {
        timeoutTimer->stop();
        timeoutTimer->deleteLater();
    });

    timeoutTimer->start(); // 启动超时定时器
}

// 处理API返回结果
void SmsHelper::slot_replyFinished(QNetworkReply* reply)
{
    // 网络错误处理
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = QString("短信发送失败：%1（错误码：%2）").arg(reply->errorString()).arg(reply->error());
        emit sendResult(false, errorMsg);
        reply->deleteLater();
        return;
    }

    // 解析返回的JSON数据
    QByteArray data = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit sendResult(false, QString("返回数据解析失败：%1").arg(parseError.errorString()));
        reply->deleteLater();
        return;
    }

    QJsonObject obj = doc.object();
    QString code = obj["Code"].toString();
    if (code == "OK") {
        emit sendResult(true, "验证码已发送至您的手机，5分钟内有效！");
    } else {
        QString errorMsg = QString("短信发送失败：%1（阿里云错误码：%2）").arg(obj["Message"].toString()).arg(code);
        emit sendResult(false, errorMsg);
    }

    reply->deleteLater();
}
