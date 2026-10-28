#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void digitClicked();
    void operatorClicked();
    void on_btnClear_clicked();
    void on_btnDecimal_clicked();
    void on_btnBackspace_clicked();
    void on_btnSign_clicked();
    void on_btnEquals_clicked();

private:
    void handleDigit(const QString &digit);
    void handleDecimal();
    void handleOperator(const QString &op);
    void handleEquals();
    void handleBackspace();
    void handleSignChange();
    void handleClear();

    double calculate(double a, double b, const QString &op) const;
    void updateDisplay();
    static QString formatNumber(double value);

    Ui::MainWindow *ui;
    QString m_currentInput;      // 当前正在输入的操作数
    QString m_expression;        // 上方表达式标签显示的内容
    double  m_firstOperand;      // 第一个操作数
    QString m_pendingOperator;   // 待执行的操作符（+ − × ÷）
    bool    m_waitingForSecond;  // 是否正在等待第二个操作数
    bool    m_justCalculated;    // 是否刚按等号得到结果
    bool    m_error;             // 是否处于错误状态（如除数为0）
};

#endif // MAINWINDOW_H
