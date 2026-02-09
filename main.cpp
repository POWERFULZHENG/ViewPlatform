#include "mainwindow.h"
#include "loginwidget.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LoginWidget loginWidget;
    loginWidget.show();
    if(loginWidget.exec() == QDialog::Accepted || loginWidget.isLoginSuccess()) {
        MainWindow w(loginWidget.m_curLoginPhone);
        w.show();
        return a.exec();
    }
    return 0;
}
