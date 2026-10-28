#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QKeyEvent>
#include <QPushButton>
#include <QList>
#include <QtMath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_currentInput(QStringLiteral("0"))
    , m_firstOperand(0.0)
    , m_waitingForSecond(false)
    , m_justCalculated(false)
    , m_error(false)
{
    ui->setupUi(this);

    const QList<QPushButton*> digitButtons = {
        ui->btn0, ui->btn1, ui->btn2, ui->btn3, ui->btn4,
        ui->btn5, ui->btn6, ui->btn7, ui->btn8, ui->btn9
    };
    for (QPushButton *btn : digitButtons) {
        connect(btn, &QPushButton::clicked, this, &MainWindow::digitClicked);
    }
    const QList<QPushButton*> operatorButtons = {
        ui->btnAdd, ui->btnSubtract, ui->btnMultiply, ui->btnDivide
    };
    for (QPushButton *btn : operatorButtons) {
        connect(btn, &QPushButton::clicked, this, &MainWindow::operatorClicked);
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

void MainWindow::operatorClicked()
{
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    if (btn) {
        handleOperator(btn->text());
    }
}

void MainWindow::on_btnClear_clicked()      { handleClear(); }
void MainWindow::on_btnDecimal_clicked()    { handleDecimal(); }
void MainWindow::on_btnBackspace_clicked()  { handleBackspace(); }
void MainWindow::on_btnSign_clicked()       { handleSignChange(); }
void MainWindow::on_btnEquals_clicked()     { handleEquals(); }

/* 键盘只做“翻译”：把按键映射成与鼠标相同的动作，
   实际处理全部调用 handleXxx()，保证两种输入逻辑完全一致。 */
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    const int key = event->key();
    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        handleDigit(QString::number(key - Qt::Key_0));
        return;
    }
    switch (key) {
    case Qt::Key_Period:
    case Qt::Key_Comma:
        handleDecimal();
        break;
    case Qt::Key_Plus:
        handleOperator(QStringLiteral("+"));
        break;
    case Qt::Key_Minus:
        handleOperator(QStringLiteral("−"));
        break;
    case Qt::Key_Asterisk:
        handleOperator(QStringLiteral("×"));
        break;
    case Qt::Key_Slash:
        handleOperator(QStringLiteral("÷"));
        break;
    case Qt::Key_Equal:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        handleEquals();
        break;
    case Qt::Key_Backspace:
        handleBackspace();
        break;
    case Qt::Key_Escape:
    case Qt::Key_Delete:
        handleClear();
        break;
    case Qt::Key_F9:
        handleSignChange();
        break;
    default:
        QMainWindow::keyPressEvent(event);
        break;
    }
}

void MainWindow::handleDigit(const QString &digit)
{
    if (m_error) {
        handleClear();
    }
    if (m_justCalculated) {
        handleClear();
        m_currentInput = digit;
    } else if (m_waitingForSecond) {
        m_currentInput = digit;
        m_waitingForSecond = false;
    } else if (m_currentInput == QStringLiteral("0")) {
        m_currentInput = digit;
    } else {
        m_currentInput += digit;
    }
    updateDisplay();
}

void MainWindow::handleDecimal()
{
    if (m_error) {
        handleClear();
    }
    if (m_justCalculated) {
        handleClear();
        m_currentInput = QStringLiteral("0.");
    } else if (m_waitingForSecond) {
        m_currentInput = QStringLiteral("0.");
        m_waitingForSecond = false;
    } else if (!m_currentInput.contains('.')) {
        m_currentInput += QStringLiteral(".");
    }
    updateDisplay();
}

void MainWindow::handleOperator(const QString &op)
{
    if (m_error) {
        return;
    }
    if (m_justCalculated) {
        m_firstOperand = m_currentInput.toDouble();
        m_pendingOperator = op;
        m_justCalculated = false;
        m_waitingForSecond = true;
    } else if (m_waitingForSecond) {
        m_pendingOperator = op;
    } else {
        const double currentValue = m_currentInput.toDouble();
        if (!m_pendingOperator.isEmpty()) {
            if (m_pendingOperator == QStringLiteral("÷") && qFuzzyIsNull(currentValue)) {
                m_error = true;
                m_expression = formatNumber(m_firstOperand) + QStringLiteral(" ÷ 0");
                m_currentInput = QStringLiteral("除数不能为0");
                updateDisplay();
                return;
            }
            m_firstOperand = calculate(m_firstOperand, currentValue, m_pendingOperator);
            m_currentInput = formatNumber(m_firstOperand);
        } else {
            m_firstOperand = currentValue;
        }
        m_pendingOperator = op;
        m_waitingForSecond = true;
    }
    updateDisplay();
}

void MainWindow::handleEquals()
{
    if (m_error) {
        return;
    }
    if (m_pendingOperator.isEmpty() || m_waitingForSecond) {
        return;
    }
    const double secondOperand = m_currentInput.toDouble();
    if (m_pendingOperator == QStringLiteral("÷") && qFuzzyIsNull(secondOperand)) {
        m_error = true;
        m_expression = formatNumber(m_firstOperand) + QStringLiteral(" ÷ 0 =");
        m_currentInput = QStringLiteral("除数不能为0");
        updateDisplay();
        return;
    }
    const double result = calculate(m_firstOperand, secondOperand, m_pendingOperator);
    m_expression = formatNumber(m_firstOperand) + QStringLiteral(" ") + m_pendingOperator
                 + QStringLiteral(" ") + formatNumber(secondOperand) + QStringLiteral(" =");
    m_currentInput = formatNumber(result);
    m_pendingOperator.clear();
    m_waitingForSecond = false;
    m_justCalculated = true;
    updateDisplay();
}

void MainWindow::handleBackspace()
{
    if (m_error || m_waitingForSecond || m_justCalculated) {
        return;
    }
    m_currentInput.chop(1);
    if (m_currentInput.isEmpty() || m_currentInput == QStringLiteral("-")) {
        m_currentInput = QStringLiteral("0");
    }
    updateDisplay();
}

void MainWindow::handleSignChange()
{
    if (m_error || m_currentInput == QStringLiteral("0")) {
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
    m_expression.clear();
    m_firstOperand = 0.0;
    m_pendingOperator.clear();
    m_waitingForSecond = false;
    m_justCalculated = false;
    m_error = false;
    updateDisplay();
}

double MainWindow::calculate(double a, double b, const QString &op) const
{
    if (op == QStringLiteral("+")) return a + b;
    if (op == QStringLiteral("−")) return a - b;
    if (op == QStringLiteral("×")) return a * b;
    if (op == QStringLiteral("÷")) return a / b;
    return 0.0;
}

void MainWindow::updateDisplay()
{
    ui->labelDisplay->setText(m_currentInput);
    if (m_error) {
        ui->labelExpression->setText(m_expression);
        return;
    }
    if (m_justCalculated) {
        ui->labelExpression->setText(m_expression);
    } else if (!m_pendingOperator.isEmpty()) {
        ui->labelExpression->setText(formatNumber(m_firstOperand) + QStringLiteral(" ") + m_pendingOperator);
    } else {
        ui->labelExpression->clear();
    }
}

QString MainWindow::formatNumber(double value)
{
    return QString::number(value, 'g', 15);
}
