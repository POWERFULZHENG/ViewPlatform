QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    apiconfigdialog.cpp \
    basedbhelper.cpp \
    baseeditdialog.cpp \
    configwidget.cpp \
    iphelper.cpp \
    llmwidget.cpp \
    logdbhelper.cpp \
    loghelper.cpp \
    loginwidget.cpp \
    logmanager.cpp \
    logtablewidget.cpp \
    main.cpp \
    mainwindow.cpp \
    personcenterwidget.cpp \
    pythonrunner.cpp \
    tableoperatewidget.cpp \
    testdbhelper.cpp \
    testtablemodel.cpp \
    userdbhelper.cpp \
    usereditdialog.cpp \
    usertablewidget.cpp

HEADERS += \
    apiconfigdialog.h \
    basedbhelper.h \
    baseeditdialog.h \
    configwidget.h \
    iphelper.h \
    llmwidget.h \
    logdbhelper.h \
    loghelper.h \
    loginwidget.h \
    logmanager.h \
    logtablewidget.h \
    mainwindow.h \
    personcenterwidget.h \
    pythonrunner.h \
    tableoperatewidget.h \
    testdbhelper.h \
    testtablemodel.h \
    userdbhelper.h \
    usereditdialog.h \
    usertablewidget.h

FORMS += \
    configwidget.ui \
    mainwindow.ui

TRANSLATIONS += \
    MyQtCreator_en_AS.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icon.qrc
