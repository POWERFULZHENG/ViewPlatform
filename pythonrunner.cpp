#include "PythonRunner.h"
#include <QDir>
#include <QDateTime>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDebug>

PythonRunner::PythonRunner(QObject *parent) : QObject(parent) {
    m_process = new QProcess(this);

    // 显式指定finished信号的重载类型
    typedef void (QProcess::*FinishedSignal)(int, QProcess::ExitStatus);
    FinishedSignal signal = &QProcess::finished;
    connect(m_process, signal, this, &PythonRunner::onProcessFinished);

    connect(m_process, &QProcess::readyReadStandardOutput, this, &PythonRunner::onReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &PythonRunner::onReadyReadStandardError);
}

PythonRunner::~PythonRunner() {
    stop();
}

void PythonRunner::setScriptPath(const QString& path) {
    m_scriptPath = path;
}

void PythonRunner::setScriptParams(const QJsonObject& params) {
    m_scriptParams = params;
}

bool PythonRunner::start() {
    if (m_scriptPath.isEmpty()) {
        emit logOutput("错误：Python脚本路径未设置");
        return false;
    }

    // 创建结果文件夹
    m_resultPath = createResultFolder();
    if (m_resultPath.isEmpty()) {
        emit logOutput("错误：创建结果文件夹失败");
        return false;
    }

    // 将参数写入JSON文件（供Python脚本读取）
    QString paramsPath = m_resultPath + "/params.json";
    QJsonDocument doc(m_scriptParams);
    QFile paramsFile(paramsPath);
    if (!paramsFile.open(QIODevice::WriteOnly)) {
        emit logOutput("错误：无法写入参数文件：" + paramsPath);
        return false;
    }
    paramsFile.write(doc.toJson(QJsonDocument::Indented));
    paramsFile.close();

    // 构造Python执行命令
    QString pythonExe = "python"; // 若系统未配置环境变量，需指定完整路径（如C:/Python39/python.exe）
    QStringList args = {
        m_scriptPath,
        "--params_path", paramsPath,
        "--result_path", m_resultPath
    };

    emit logOutput("启动Python脚本：" + pythonExe + " " + args.join(" "));
    m_process->start(pythonExe, args);

    return m_process->waitForStarted(3000);
}

void PythonRunner::stop() {
    if (m_process->state() == QProcess::Running) {
        m_process->kill();
        m_process->waitForFinished();
        emit logOutput("Python脚本已终止");
    }
}

void PythonRunner::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    QDateTime execTime = QDateTime::currentDateTime();
    QString metricsData = "";

    if (exitStatus == QProcess::CrashExit || exitCode != 0) {
        emit logOutput("错误：Python脚本执行失败，退出码：" + QString::number(exitCode));
        emit finished(false, execTime, metricsData);
        return;
    }

    // 读取指标数据（假设Python脚本输出metrics.json到结果文件夹）
    QString metricsPath = m_resultPath + "/metrics.json";
    QFile metricsFile(metricsPath);
    if (metricsFile.open(QIODevice::ReadOnly)) {
        QByteArray data = metricsFile.readAll();
        metricsData = QString(data);
        metricsFile.close();
        emit logOutput("成功读取指标数据：" + metricsPath);
    } else {
        emit logOutput("警告：未找到指标文件：" + metricsPath);
    }

    emit logOutput("Python脚本执行完成，结果路径：" + m_resultPath);
    emit finished(true, execTime, metricsData);
}

void PythonRunner::onReadyReadStandardOutput() {
    QString output = QString::fromUtf8(m_process->readAllStandardOutput());
    emit logOutput("Python输出：" + output);
}

void PythonRunner::onReadyReadStandardError() {
    QString error = QString::fromUtf8(m_process->readAllStandardError());
    emit logOutput("Python错误：" + error);
}

QString PythonRunner::createResultFolder() {
    // 生成唯一的文件夹名称（基于时间）
    QString timeStr = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString basePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/TestResults";
    QString folderPath = basePath + "/" + timeStr;

    QDir dir;
    if (!dir.mkpath(folderPath)) {
        return "";
    }

    return folderPath;
}
