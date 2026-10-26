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
    void on_btnClear_clicked();

private:
    void handleDigit(const QString &digit);
    void handleClear();
    void updateDisplay();

    Ui::MainWindow *ui;
    QString m_currentInput;   // 当前正在输入的操作数
};

#endif // MAINWINDOW_H
