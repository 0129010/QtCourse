QT       += core gui sql widgets

TEMPLATE = app
TARGET   = Lab3
CONFIG  += c++11

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += .

SOURCES += \
    main.cpp \
    db/databasemanager.cpp \
    db/patienttablemodel.cpp \
    ui/logindialog.cpp \
    ui/mainwindow.cpp \
    ui/patientpage.cpp \
    ui/patientdialog.cpp \
    ui/doctorpage.cpp \
    ui/departmentpage.cpp \
    ui/historypage.cpp \
    ui/delegate/comboboxdelegate.cpp \
    ui/delegate/doublespindelegate.cpp \
    util/messagebox.cpp

HEADERS += \
    db/databasemanager.h \
    db/patienttablemodel.h \
    ui/logindialog.h \
    ui/mainwindow.h \
    ui/patientpage.h \
    ui/patientdialog.h \
    ui/doctorpage.h \
    ui/departmentpage.h \
    ui/historypage.h \
    ui/delegate/comboboxdelegate.h \
    ui/delegate/doublespindelegate.h \
    util/messagebox.h
