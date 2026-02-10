#ifndef LOGHELPER_H
#define LOGHELPER_H

#include <QDebug>
#include <QString>
#include <QMutex>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QSet>
#include <QRegularExpression>

// ========== 基础日志级别 ==========
enum LogLevel {
    LOG_LEVEL_OFF     = 0, // 关闭所有日志
    LOG_LEVEL_FATAL   = 1, // 致命错误（触发程序终止）
    LOG_LEVEL_ERROR   = 2, // 普通错误
    LOG_LEVEL_WARN    = 3, // 警告
    LOG_LEVEL_INFO    = 4, // 业务信息
    LOG_LEVEL_DEBUG   = 5  // 调试日志（默认）
};

// ========== 新增：日志分割配置 ==========
// 分割类型
enum LogSplitType {
    SPLIT_NONE,       // 不分割
    SPLIT_BY_SIZE,    // 按文件大小分割
    SPLIT_BY_TIME,    // 按时间分割（小时/天）
    SPLIT_BY_BOTH     // 按大小+时间（满足其一即分割）
};

// 时间分割单位
enum LogTimeSplitUnit {
    SPLIT_HOUR,       // 每小时分割
    SPLIT_DAY,        // 每天分割
    SPLIT_WEEK        // 每周分割
};

// ========== 新增：日志过滤配置 ==========
// 过滤模式（白名单/黑名单）
enum LogFilterMode {
    FILTER_WHITE_LIST,// 仅打印匹配的日志
    FILTER_BLACK_LIST // 屏蔽匹配的日志
};

// ========== 新增：自定义格式配置 ==========
struct LogFormatConfig {
    bool showTime = true;        // 是否显示时间
    bool showLevel = true;       // 是否显示日志级别
    bool showFile = true;        // 是否显示文件（仅文件名/全路径）
    bool showFullFilePath = false;// showFile=true时，是否显示全路径（默认仅文件名）
    bool showLine = true;        // 是否显示行号
    bool showModule = true;      // 是否显示模块名（需日志宏传入模块）
    QString timeFormat = "yyyy-MM-dd hh:mm:ss.zzz"; // 时间格式
};

/**
 * @brief 日志工具类（单例模式）
 * 扩展功能：
 * 1. 日志分割（按大小/时间/大小+时间）
 * 2. 日志过滤（关键词/模块，白名单/黑名单）
 * 3. 自定义输出格式
 * 4. 对接系统日志（Windows事件日志/Linux syslog）
 */
class LogHelper
{
public:
    // 获取单例实例
    static LogHelper& instance();

    // ========== 基础日志级别控制 ==========
    void setLogLevel(LogLevel level);
    LogLevel getLogLevel() const;

    // ========== 日志文件 & 分割配置 ==========
    void setLogFile(const QString& filePath);
    // 设置日志分割规则
    void setLogSplitRule(LogSplitType type,
                         qint64 maxSizeMB = 100,  // 按大小分割：阈值(MB)
                         LogTimeSplitUnit timeUnit = SPLIT_DAY); // 按时间分割：单位
    // ========== 日志过滤配置 ==========
    // 设置过滤模式（白名单/黑名单）
    void setFilterMode(LogFilterMode mode);
    // 添加过滤关键词（支持正则）
    void addFilterKeyword(const QString& keyword, bool isRegex = false);
    // 清空关键词过滤
    void clearFilterKeywords();
    // 添加过滤模块
    void addFilterModule(const QString& module);
    // 清空模块过滤
    void clearFilterModules();
    // ========== 自定义格式配置 ==========
    void setLogFormat(const LogFormatConfig& format);
    LogFormatConfig getLogFormat() const;
    // ========== 系统日志对接 ==========
    // 启用/禁用系统日志输出（Windows事件日志/Linux syslog）
    void enableSystemLog(bool enable);

    // ========== 日志打印核心方法（支持模块名） ==========
    void printLog(LogLevel level, const QString& module, const QString& file, int line, const QString& msg);

private:
    LogHelper();
    ~LogHelper();
    LogHelper(const LogHelper&) = delete;
    LogHelper& operator=(const LogHelper&) = delete;

    // ========== 内部辅助方法 ==========
    // 检查并分割日志文件（内部调用）
    void checkAndSplitLogFile();
    // 生成分割后的日志文件名（如 log_20260210_15.log）
    QString generateSplitFileName();
    // 过滤日志（返回true=需要打印，false=过滤掉）
    bool filterLog(const QString& module, const QString& msg);
    // 拼接自定义格式的日志头部
    QString buildLogHeader(LogLevel level, const QString& module, const QString& file, int line);
    // 输出到系统日志（Windows/Linux 分别实现）
    void writeToSystemLog(LogLevel level, const QString& fullMsg);

    // ========== 成员变量 ==========
    // 基础配置
    LogLevel  m_logLevel = LOG_LEVEL_DEBUG;
    mutable QMutex m_logMutex;
    // 文件配置
    QString   m_logFilePath;
    QFile*    m_logFile = nullptr;
    qint64    m_logFileSize = 0; // 当前日志文件大小
    // 分割配置
    LogSplitType m_splitType = SPLIT_NONE;
    qint64    m_maxSizeMB = 100; // 最大文件大小（MB）
    LogTimeSplitUnit m_timeUnit = SPLIT_DAY;
    QDateTime m_lastSplitTime;   // 上次分割时间
    // 过滤配置
    LogFilterMode m_filterMode = FILTER_BLACK_LIST;
    QSet<QString> m_filterKeywords; // 过滤关键词
    QSet<QRegularExpression> m_regexKeywords; // 正则关键词
    QSet<QString> m_filterModules;  // 过滤模块
    // 格式配置
    LogFormatConfig m_formatConfig;
    // 系统日志配置
    bool      m_enableSystemLog = false;
};

// ========== 日志宏封装（新增模块参数） ==========
#ifdef QT_DEBUG
#define LOG_DEBUG(module, content) \
    do { \
        if (LogHelper::instance().getLogLevel() >= LOG_LEVEL_DEBUG) { \
            QString msg; QDebug(&msg) << content; \
            LogHelper::instance().printLog(LOG_LEVEL_DEBUG, module, __FILE__, __LINE__, msg); \
        } \
    } while(0)
#else
#define LOG_DEBUG(module, content) ((void)0)
#endif

#define LOG_INFO(module, content) \
    do { \
        if (LogHelper::instance().getLogLevel() >= LOG_LEVEL_INFO) { \
            QString msg; QDebug(&msg) << content; \
            LogHelper::instance().printLog(LOG_LEVEL_INFO, module, __FILE__, __LINE__, msg); \
        } \
    } while(0)

#define LOG_WARN(module, content) \
    do { \
        if (LogHelper::instance().getLogLevel() >= LOG_LEVEL_WARN) { \
            QString msg; QDebug(&msg) << content; \
            LogHelper::instance().printLog(LOG_LEVEL_WARN, module, __FILE__, __LINE__, msg); \
        } \
    } while(0)

#define LOG_ERROR(module, content) \
    do { \
        if (LogHelper::instance().getLogLevel() >= LOG_LEVEL_ERROR) { \
            QString msg; QDebug(&msg) << content; \
            LogHelper::instance().printLog(LOG_LEVEL_ERROR, module, __FILE__, __LINE__, msg); \
        } \
    } while(0)

#define LOG_FATAL(module, content) \
    do { \
        QString msg; QDebug(&msg) << content; \
        LogHelper::instance().printLog(LOG_LEVEL_FATAL, module, __FILE__, __LINE__, msg); \
        abort(); \
    } while(0)

// ========== 全局配置宏 ==========
#define SET_LOG_LEVEL(level) \
    do { \
        LogHelper::instance().setLogLevel(level); \
    } while(0)

#define SET_LOG_FILE(path) \
    do { \
        LogHelper::instance().setLogFile(path); \
    } while(0)

// 设置日志分割规则
#define SET_LOG_SPLIT_RULE(type, maxSizeMB, timeUnit) \
    do { \
        LogHelper::instance().setLogSplitRule(type, maxSizeMB, timeUnit); \
    } while(0)

// 设置过滤模式
#define SET_LOG_FILTER_MODE(mode) \
    do { \
        LogHelper::instance().setFilterMode(mode); \
    } while(0)

// 添加过滤关键词
#define ADD_LOG_FILTER_KEYWORD(keyword, isRegex) \
    do { \
        LogHelper::instance().addFilterKeyword(keyword, isRegex); \
    } while(0)

// 添加过滤模块
#define ADD_LOG_FILTER_MODULE(module) \
    do { \
        LogHelper::instance().addFilterModule(module); \
    } while(0)

// 设置自定义格式
#define SET_LOG_FORMAT(format) \
    do { \
        LogHelper::instance().setLogFormat(format); \
    } while(0)

// 启用系统日志
#define ENABLE_SYSTEM_LOG(enable) \
    do { \
        LogHelper::instance().enableSystemLog(enable); \
    } while(0)

#endif // LOGHELPER_H
