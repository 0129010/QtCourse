#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QAction>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QAction *actionAbout = new QAction("关于", this);
    connect(actionAbout, &QAction::triggered, this, &MainWindow::onShowAbout);
    ui->toolBar->addAction(actionAbout);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onShowAbout()
{
    QMessageBox::about(this, "关于程序",
        "Qt第2周作业\n"
        "姓名：高俊\n"
        "学号：2024414300103\n"
        "功能：工具栏关于按钮弹出信息框");
}
