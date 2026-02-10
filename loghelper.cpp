#include "loghelper.h"
#include <QDir>
#include <QFileInfo>
#include <QRegularExpressionMatch>

// ========== 系统日志对接（条件编译） ==========
#ifdef Q_OS_WIN
#include <windows.h>
#include <winbase.h>
#else
#include <syslog.h>
#endif

// ========== 单例实现 ==========
LogHelper& LogHelper::instance()
{
    static LogHelper s_instance;
    return s_instance;
}

// ========== 构造/析构 ==========
LogHelper::LogHelper()
{
    // 初始化分割时间为当前时间
    m_lastSplitTime = QDateTime::currentDateTime();
    // 默认格式配置
    m_formatConfig = LogFormatConfig();
}

LogHelper::~LogHelper()
{
    QMutexLocker locker(&m_logMutex);
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
        m_logFile = nullptr;
    }
}

// ========== 基础日志级别控制 ==========
void LogHelper::setLogLevel(LogLevel level)
{
    QMutexLocker locker(&m_logMutex);
    m_logLevel = level;
}

LogLevel LogHelper::getLogLevel() const
{
    QMutexLocker locker(&m_logMutex);
    return m_logLevel;
}

// ========== 日志文件 & 分割配置 ==========
void LogHelper::setLogFile(const QString& filePath)
{
    QMutexLocker locker(&m_logMutex);
    m_logFilePath = filePath;

    if (m_logFilePath.isEmpty()) {
        if (m_logFile) {
            m_logFile->close();
            delete m_logFile;
            m_logFile = nullptr;
        }
        return;
    }

    // 创建目录
    QDir dir = QFileInfo(m_logFilePath).dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // 关闭旧文件
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
        m_logFile = nullptr;
    }

    // 打开新文件（先检查分割）
    m_logFile = new QFile(m_logFilePath);

    if (m_logFile->open(QIODevice::Append | QIODevice::Text)) {
        m_logFileSize = m_logFile->size(); // 初始化文件大小
    } else {
        qCritical() << "Failed to open log file:" << m_logFilePath;
        delete m_logFile;
        m_logFile = nullptr;
    }
}

void LogHelper::setLogSplitRule(LogSplitType type, qint64 maxSizeMB, LogTimeSplitUnit timeUnit)
{
    QMutexLocker locker(&m_logMutex);
    m_splitType = type;
    m_maxSizeMB = maxSizeMB;
    m_timeUnit = timeUnit;
}

// ========== 日志分割辅助方法 ==========
QString LogHelper::generateSplitFileName()
{
    QFileInfo fi(m_logFilePath);
    QString baseName = fi.baseName();
    QString suffix = fi.suffix();
    QString dir = fi.path();

    // 生成时间戳（如 20260210_1530）
    QString timeStamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    // 新文件名：log_20260210_1530.log
    return QString("%1/%2_%3.%4").arg(dir).arg(baseName).arg(timeStamp).arg(suffix);
}

void LogHelper::checkAndSplitLogFile()
{
    if (!m_logFile || !m_logFile->isOpen() || m_splitType == SPLIT_NONE) {
        return;
    }

    bool needSplit = false;
    qint64 maxSizeBytes = m_maxSizeMB * 1024 * 1024; // 转换为字节

    // 1. 按大小检查
    if (m_splitType == SPLIT_BY_SIZE || m_splitType == SPLIT_BY_BOTH) {
        if (m_logFileSize >= maxSizeBytes) {
            needSplit = true;
        }
    }

    // 2. 按时间检查
    if (!needSplit && (m_splitType == SPLIT_BY_TIME || m_splitType == SPLIT_BY_BOTH)) {
        QDateTime now = QDateTime::currentDateTime();
        qint64 secsDiff = m_lastSplitTime.secsTo(now);

        switch (m_timeUnit) {
        case SPLIT_HOUR:
            needSplit = (secsDiff >= 3600); // 1小时
            break;
        case SPLIT_DAY:
            needSplit = (secsDiff >= 86400); // 1天
            break;
        case SPLIT_WEEK:
            needSplit = (secsDiff >= 604800); // 1周
            break;
        }
    }

    // 3. 执行分割
    if (needSplit) {
        m_logFile->close();
        // 重命名旧文件
        QString newFileName = generateSplitFileName();
        QFile::rename(m_logFilePath, newFileName);
        // 重新打开新文件
        m_logFile->setFileName(m_logFilePath);
        if (m_logFile->open(QIODevice::Append | QIODevice::Text)) {
            m_logFileSize = 0; // 重置文件大小
            m_lastSplitTime = QDateTime::currentDateTime(); // 更新分割时间
        } else {
            qCritical() << "Failed to re-open log file after split:" << m_logFilePath;
        }
    }
}

// ========== 日志过滤配置 ==========
void LogHelper::setFilterMode(LogFilterMode mode)
{
    QMutexLocker locker(&m_logMutex);
    m_filterMode = mode;
}

void LogHelper::addFilterKeyword(const QString& keyword, bool isRegex)
{
    QMutexLocker locker(&m_logMutex);
    if (isRegex) {
        m_regexKeywords.insert(QRegularExpression(keyword));
    } else {
        m_filterKeywords.insert(keyword);
    }
}

void LogHelper::clearFilterKeywords()
{
    QMutexLocker locker(&m_logMutex);
    m_filterKeywords.clear();
    m_regexKeywords.clear();
}

void LogHelper::addFilterModule(const QString& module)
{
    QMutexLocker locker(&m_logMutex);
    m_filterModules.insert(module);
}

void LogHelper::clearFilterModules()
{
    QMutexLocker locker(&m_logMutex);
    m_filterModules.clear();
}

// 过滤逻辑：返回true=需要打印，false=过滤掉
bool LogHelper::filterLog(const QString& module, const QString& msg)
{
    if (m_filterKeywords.isEmpty() && m_regexKeywords.isEmpty() && m_filterModules.isEmpty()) {
        return true; // 无过滤规则，全部打印
    }

    bool match = false;

    // 1. 模块过滤
    if (!m_filterModules.isEmpty()) {
        match = m_filterModules.contains(module);
    }

    // 2. 关键词过滤（正则+普通）
    if (!match && !m_filterKeywords.isEmpty()) {
        for (const QString& kw : m_filterKeywords) {
            if (msg.contains(kw, Qt::CaseInsensitive)) {
                match = true;
                break;
            }
        }
    }

    // 3. 正则关键词过滤
    if (!match && !m_regexKeywords.isEmpty()) {
        for (const QRegularExpression& re : m_regexKeywords) {
            if (re.match(msg).hasMatch()) {
                match = true;
                break;
            }
        }
    }

    // 白名单：仅匹配的打印；黑名单：匹配的不打印
    return (m_filterMode == FILTER_WHITE_LIST) ? match : !match;
}

// ========== 自定义格式配置 ==========
void LogHelper::setLogFormat(const LogFormatConfig& format)
{
    QMutexLocker locker(&m_logMutex);
    m_formatConfig = format;
}

LogFormatConfig LogHelper::getLogFormat() const
{
    QMutexLocker locker(&m_logMutex);
    return m_formatConfig;
}

// 拼接自定义格式头部
QString LogHelper::buildLogHeader(LogLevel level, const QString& module, const QString& file, int line)
{
    QStringList headerParts;

    // 1. 日志级别
    if (m_formatConfig.showLevel) {
        QString levelStr;
        switch (level) {
        case LOG_LEVEL_DEBUG: levelStr = "[DEBUG]"; break;
        case LOG_LEVEL_INFO:  levelStr = "[INFO]";  break;
        case LOG_LEVEL_WARN:  levelStr = "[WARN]";  break;
        case LOG_LEVEL_ERROR: levelStr = "[ERROR]"; break;
        case LOG_LEVEL_FATAL: levelStr = "[FATAL]"; break;
        default:              levelStr = "[UNKNOWN]";
        }
        headerParts << levelStr;
    }

    // 2. 时间
    if (m_formatConfig.showTime) {
        QString timeStr = QDateTime::currentDateTime().toString(m_formatConfig.timeFormat);
        headerParts << timeStr;
    }

    // 3. 模块名
    if (m_formatConfig.showModule && !module.isEmpty()) {
        headerParts << QString("[%1]").arg(module);
    }

    // 4. 文件+行号
    if (m_formatConfig.showFile) {
        QString filePath = m_formatConfig.showFullFilePath ? file : QFileInfo(file).fileName();
        if (m_formatConfig.showLine) {
            headerParts << QString("%1:%2").arg(filePath).arg(line);
        } else {
            headerParts << filePath;
        }
    }

    return headerParts.join(" ") + " | ";
}

// ========== 系统日志对接 ==========
void LogHelper::enableSystemLog(bool enable)
{
    QMutexLocker locker(&m_logMutex);
    m_enableSystemLog = enable;

#ifdef Q_OS_LINUX
    if (enable) {
        openlog("ViewPlatform", LOG_PID | LOG_CONS, LOG_USER); // 初始化syslog
    } else {
        closelog(); // 关闭syslog
    }
#endif
}

void LogHelper::writeToSystemLog(LogLevel level, const QString& fullMsg)
{
    if (!m_enableSystemLog) {
        return;
    }

#ifdef Q_OS_WIN
    // Windows 事件日志
    HANDLE hEventLog = RegisterEventSourceA(nullptr, "ViewPlatform");
    if (hEventLog) {
        WORD type;
        switch (level) {
        case LOG_LEVEL_FATAL:
        case LOG_LEVEL_ERROR: type = EVENTLOG_ERROR_TYPE; break;
        case LOG_LEVEL_WARN:  type = EVENTLOG_WARNING_TYPE; break;
        default:              type = EVENTLOG_INFORMATION_TYPE;
        }
        const char* msg = fullMsg.toUtf8().constData();
        ReportEventA(hEventLog, type, 0, 0, nullptr, 1, 0, &msg, nullptr);
        DeregisterEventSource(hEventLog);
    }
#else
    // Linux syslog
    int priority;
    switch (level) {
    case LOG_LEVEL_FATAL: priority = LOG_CRIT; break;
    case LOG_LEVEL_ERROR: priority = LOG_ERR; break;
    case LOG_LEVEL_WARN:  priority = LOG_WARNING; break;
    case LOG_LEVEL_INFO:  priority = LOG_INFO; break;
    default:              priority = LOG_DEBUG;
    }
    syslog(priority, "%s", fullMsg.toUtf8().constData());
#endif
}

// ========== 核心打印方法 ==========
void LogHelper::printLog(LogLevel level, const QString& module, const QString& file, int line, const QString& msg)
{
    QMutexLocker locker(&m_logMutex);

    // 1. 级别过滤（基础过滤）
    if (level > m_logLevel || m_logLevel == LOG_LEVEL_OFF) {
        return;
    }

    // 2. 关键词/模块过滤
    if (!filterLog(module, msg)) {
        return;
    }

    // 3. 构建自定义格式日志
    QString header = buildLogHeader(level, module, file, line);
    QString fullMsg = header + msg;

    // 4. 控制台输出
    switch (level) {
    case LOG_LEVEL_DEBUG: qDebug().noquote() << fullMsg; break;
    case LOG_LEVEL_INFO:  qInfo().noquote()  << fullMsg; break;
    case LOG_LEVEL_WARN:  qWarning().noquote()<< fullMsg; break;
    case LOG_LEVEL_ERROR: qCritical().noquote()<< fullMsg; break;
    case LOG_LEVEL_FATAL: qFatal("%s", fullMsg.toUtf8().constData()); break;
    default: qDebug().noquote() << fullMsg;
    }

    // 5. 系统日志输出
    writeToSystemLog(level, fullMsg);

    // 6. 文件输出（先检查分割）
    if (m_logFile && m_logFile->isOpen()) {
        checkAndSplitLogFile(); // 检查是否需要分割

        QTextStream stream(m_logFile);
        stream << fullMsg << "\n";
        stream.flush();

        // 更新文件大小
        m_logFileSize = m_logFile->size();
    }
}
