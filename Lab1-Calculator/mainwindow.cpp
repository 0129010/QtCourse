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

void MainWindow::on_btnClear_clicked()      { handleClear(); }
void MainWindow::on_btnDecimal_clicked()    { handleDecimal(); }
void MainWindow::on_btnBackspace_clicked()  { handleBackspace(); }
void MainWindow::on_btnSign_clicked()       { handleSignChange(); }

void MainWindow::handleDigit(const QString &digit)
{
    if (m_currentInput == QStringLiteral("0")) {
        m_currentInput = digit;
    } else {
        m_currentInput += digit;
    }
    updateDisplay();
}

void MainWindow::handleDecimal()
{
    // 同一操作数中只允许出现一个小数点
    if (!m_currentInput.contains('.')) {
        m_currentInput += QStringLiteral(".");
    }
    updateDisplay();
}

void MainWindow::handleBackspace()
{
    m_currentInput.chop(1);
    if (m_currentInput.isEmpty() || m_currentInput == QStringLiteral("-")) {
        m_currentInput = QStringLiteral("0");
    }
    updateDisplay();
}

void MainWindow::handleSignChange()
{
    if (m_currentInput == QStringLiteral("0")) {
        return;
    }
    if (m_currentInput.startsWith('-')) {
        m_currentInput.remove(0, 1);
    } else {
        m_currentInput.prepend('-');
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
