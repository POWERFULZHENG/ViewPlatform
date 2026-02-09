#ifndef ALGORITHMSDATAWIDGET_H
#define ALGORITHMSDATAWIDGET_H

#include <QWidget>
#include <QMainWindow>
#include "ui_ConfigWidget.h"
#include "TestDbHelper.h"
#include "ConfigWidget.h"
#include "PythonRunner.h"
#include "TestTableModel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class AlgorithmsDataWidget : public QWidget
{
    Q_OBJECT
public:
    eplicit AlgorithmsDataWidget(Ui::AlgorithmsDataWidget* ui, QAlgorithmsDataWidget *AlgorithmsDataWidget, QWidget *parent);


signals:
private:
    QMainWindow *m_mainWindow;
    Ui::MainWindow *m_ui;
    TestDbHelper *m_testDbHelper;
    PythonRunner *m_pythonRunner;
    ConfigWidget *m_configWidget;
    TestTableModel *m_testTableModel;
};

#endif // ALGORITHMSDATAWIDGET_H
