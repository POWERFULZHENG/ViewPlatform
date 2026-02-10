#include "mainwindow.h"
#include "loginwidget.h"
#include <QApplication>
#include "loghelper.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // ========== 全局日志配置 ==========
    // 1. 设置日志级别（发布期设为INFO，开发期设为DEBUG）
    SET_LOG_LEVEL(LOG_LEVEL_DEBUG);

    // 2. 设置日志文件+分割规则
    QString logDir = QCoreApplication::applicationDirPath() + "/logs";
    QString logFilePath = logDir + "/view_platform.log";
    SET_LOG_FILE(logFilePath);
    SET_LOG_SPLIT_RULE(SPLIT_BY_BOTH, 50, SPLIT_DAY); // 50MB/每天分割

    // 3. 日志过滤（屏蔽包含"密码"的日志，仅打印登录/配置模块）
    SET_LOG_FILTER_MODE(FILTER_WHITE_LIST);
    ADD_LOG_FILTER_MODULE("登录模块");
    ADD_LOG_FILTER_MODULE("日志模块");
    ADD_LOG_FILTER_MODULE("配置模块");
    ADD_LOG_FILTER_MODULE("LLM模块");
    ADD_LOG_FILTER_MODULE("主窗口模块");
    ADD_LOG_FILTER_MODULE("用户信息模块");
    ADD_LOG_FILTER_MODULE("个人中心模块");
    ADD_LOG_FILTER_KEYWORD("密码", false);

    // 4. 自定义格式
    LogFormatConfig format;
    format.showTime = true;
    format.showLevel = true;
    format.showModule = true;
    format.showFile = true;
    format.showLine = true;
    format.showFullFilePath = false;
    SET_LOG_FORMAT(format);

    // 5. 启用系统日志
    ENABLE_SYSTEM_LOG(true);

    LoginWidget loginWidget;
    loginWidget.show();
    if(loginWidget.exec() == QDialog::Accepted || loginWidget.isLoginSuccess()) {
        MainWindow w(loginWidget.m_curLoginPhone);
        w.show();
        return a.exec();
    }
    return 0;
}
