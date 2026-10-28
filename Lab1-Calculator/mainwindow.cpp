#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QKeyEvent>
#include <QPushButton>
#include <QtMath>
#include <QList>

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

    // 显示区固定在顶部，按钮区域占据剩余空间
    ui->mainLayout->setStretch(0, 0);
    ui->mainLayout->setStretch(1, 1);

    // 让所有按钮在网格中水平、垂直方向都能随窗口拉伸
    const QList<QPushButton*> allButtons = {
        ui->btn0, ui->btn1, ui->btn2, ui->btn3, ui->btn4,
        ui->btn5, ui->btn6, ui->btn7, ui->btn8, ui->btn9,
        ui->btnAdd, ui->btnSubtract, ui->btnMultiply, ui->btnDivide,
        ui->btnDecimal, ui->btnEquals, ui->btnClear,
        ui->btnBackspace, ui->btnSign
    };
    for (QPushButton *btn : allButtons) {
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    // 数字按钮统一连接到 digitClicked，通过 sender() 区分具体按键
    const QList<QPushButton*> digitButtons = {
        ui->btn0, ui->btn1, ui->btn2, ui->btn3, ui->btn4,
        ui->btn5, ui->btn6, ui->btn7, ui->btn8, ui->btn9
    };
    for (QPushButton *btn : digitButtons) {
        connect(btn, &QPushButton::clicked, this, &MainWindow::digitClicked);
    }

    // 四则运算符按钮统一连接到 operatorClicked
    const QList<QPushButton*> operatorButtons = {
        ui->btnAdd, ui->btnSubtract, ui->btnMultiply, ui->btnDivide
    };
    for (QPushButton *btn : operatorButtons) {
        connect(btn, &QPushButton::clicked, this, &MainWindow::operatorClicked);
    }

    applyStyleSheet();
    handleClear();   // 初始化显示状态
}

MainWindow::~MainWindow()
{
    delete ui;
}

/* ===================== 鼠标点击槽函数 ===================== */

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

void MainWindow::on_btnDecimal_clicked()
{
    handleDecimal();
}

void MainWindow::on_btnEquals_clicked()
{
    handleEquals();
}

void MainWindow::on_btnClear_clicked()
{
    handleClear();
}

void MainWindow::on_btnBackspace_clicked()
{
    handleBackspace();
}

void MainWindow::on_btnSign_clicked()
{
    handleSignChange();
}

/* ===================== 键盘事件 ===================== */
/* 键盘只做“翻译”，把按键映射成与鼠标相同的动作，
   实际处理全部调用 handleXxx()，保证两种输入逻辑完全一致。 */

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    const int key = event->key();

    // 数字 0-9（主键盘以及 NumLock 开启时的小键盘）
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
    case Qt::Key_Enter:            // 小键盘回车
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

/* ===================== 核心处理逻辑（鼠标/键盘共用） ===================== */

void MainWindow::handleDigit(const QString &digit)
{
    if (m_error) {
        handleClear();   // 错误状态下输入数字，先清除错误再开始新计算
    }

    if (m_justCalculated) {
        // 计算完成后直接输入数字：开始一次全新的计算
        handleClear();
        m_currentInput = digit;
    } else if (m_waitingForSecond) {
        // 刚按过操作符，开始输入第二个操作数
        m_currentInput = digit;
        m_waitingForSecond = false;
    } else {
        // 正常追加数字，避免出现前导 0（如 007）
        if (m_currentInput == QStringLiteral("0")) {
            m_currentInput = digit;
        } else {
            m_currentInput += digit;
        }
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
        // 操作符后直接按小数点，第二个操作数从 "0." 开始
        m_currentInput = QStringLiteral("0.");
        m_waitingForSecond = false;
    } else if (!m_currentInput.contains('.')) {
        // 同一操作数中只允许出现一个小数点
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
        // 用上次的计算结果作为新的第一个操作数，继续运算
        m_firstOperand = m_currentInput.toDouble();
        m_pendingOperator = op;
        m_justCalculated = false;
        m_waitingForSecond = true;
    } else if (m_waitingForSecond) {
        // 连续按多个操作符：以最后按下的操作符为准
        m_pendingOperator = op;
    } else {
        const double currentValue = m_currentInput.toDouble();
        if (!m_pendingOperator.isEmpty()) {
            // 已存在待执行的运算（连续运算）：先算出中间结果
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
    // 没有待执行的操作符，或还没输入第二个操作数，则等号不做处理
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
    // 错误、等待第二个操作数、刚算出结果等状态下退格没有意义，直接忽略
    if (m_error || m_waitingForSecond || m_justCalculated) {
        return;
    }
    m_currentInput.chop(1);
    // 删空或只剩负号时，回到 0
    if (m_currentInput.isEmpty() || m_currentInput == QStringLiteral("-")) {
        m_currentInput = QStringLiteral("0");
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

/* ===================== 辅助函数 ===================== */

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
        // 输入过程中表达式区实时显示 “第一个操作数 操作符”
        ui->labelExpression->setText(formatNumber(m_firstOperand) + QStringLiteral(" ") + m_pendingOperator);
    } else {
        ui->labelExpression->clear();
    }
}

QString MainWindow::formatNumber(double value)
{
    // 'g' 格式自动去掉多余的 0，最多保留 15 位有效数字以减小浮点误差
    return QString::number(value, 'g', 15);
}

void MainWindow::applyStyleSheet()
{
    // 显示区字体
    ui->labelDisplay->setStyleSheet(
        QStringLiteral("font-size: 40px; font-weight: bold; color: #1c1c1e;"));
    ui->labelExpression->setStyleSheet(
        QStringLiteral("font-size: 18px; color: #8e8e93;"));

    // 整体窗口与按钮样式（参考手机计算器配色）
    const QString qss = QStringLiteral(
        "QMainWindow, QWidget#centralwidget { background-color: #f2f3f5; }"
        "QPushButton {"
        "  border: none; border-radius: 10px; font-size: 20px;"
        "  background-color: #ffffff; color: #1c1c1e;"
        "  min-height: 56px;"
        "}"
        "QPushButton:pressed { background-color: #e5e5ea; }"
        // 清除、退格：浅色底 + 彩色文字
        "QPushButton#btnClear, QPushButton#btnBackspace {"
        "  background-color: #e9f4ff; color: #0a84ff;"
        "}"
        // 四则运算符：浅蓝底 + 蓝色文字
        "QPushButton#btnAdd, QPushButton#btnSubtract,"
        "QPushButton#btnMultiply, QPushButton#btnDivide {"
        "  background-color: #d6ecff; color: #0a84ff; font-size: 24px;"
        "}"
        // 正负号、小数点
        "QPushButton#btnSign, QPushButton#btnDecimal {"
        "  background-color: #ffffff; color: #1c1c1e;"
        "}"
        // 等号：实心蓝底白字
        "QPushButton#btnEquals {"
        "  background-color: #0a84ff; color: #ffffff; font-size: 26px;"
        "}"
        "QPushButton#btnEquals:pressed { background-color: #0066cc; }"
    );
    setStyleSheet(qss);
}
