#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>
#include <QList>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_currentInput(QStringLiteral("0"))
{
    ui->setupUi(this);

    // 数字按钮统一连接到 digitClicked，通过 sender() 区分具体按键
    const QList<QPushButton*> digitButtons = {
        ui->btn0, ui->btn1, ui->btn2, ui->btn3, ui->btn4,
        ui->btn5, ui->btn6, ui->btn7, ui->btn8, ui->btn9
    };
    for (QPushButton *btn : digitButtons) {
        connect(btn, &QPushButton::clicked, this, &MainWindow::digitClicked);
    }
    handleClear();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::digitClicked()
{
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    if (btn) {
        handleDigit(btn->text());
    }
}

void MainWindow::on_btnClear_clicked()
{
    handleClear();
}

void MainWindow::handleDigit(const QString &digit)
{
    // 避免出现前导 0（如 007）
    if (m_currentInput == QStringLiteral("0")) {
        m_currentInput = digit;
    } else {
        m_currentInput += digit;
    }
    updateDisplay();
}

void MainWindow::handleClear()
{
    m_currentInput = QStringLiteral("0");
    updateDisplay();
}

void MainWindow::updateDisplay()
{
    ui->labelDisplay->setText(m_currentInput);
}
