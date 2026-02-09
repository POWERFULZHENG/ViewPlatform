#ifndef PYTHONRUNNER_H
#define PYTHONRUNNER_H

#include <QObject>
#include <QProcess>
#include <QJsonObject>
#include <QDateTime>

class PythonRunner : public QObject
{
    Q_OBJECT
public:
    explicit PythonRunner(QObject *parent = nullptr);
    ~PythonRunner();

    // 设置Python脚本路径和参数
    void setScriptPath(const QString& path);
    void setScriptParams(const QJsonObject& params);

    // 启动Python脚本
    bool start();

    // 停止Python脚本
    void stop();

    // 获取结果保存路径
    QString getResultPath() const { return m_resultPath; }

signals:
    // 脚本执行完成（成功/失败，执行时间，指标数据JSON）
    void finished(bool success, const QDateTime& execTime, const QString& metricsData);
    // 输出日志
    void logOutput(const QString& log);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();

private:
    QProcess* m_process;
    QString m_scriptPath;
    QJsonObject m_scriptParams;
    QString m_resultPath;

    // 创建结果文件夹
    QString createResultFolder();
};

#endif // PYTHONRUNNER_H
