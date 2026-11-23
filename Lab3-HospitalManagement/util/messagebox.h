#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QMessageBox>
#include <QString>

class QWidget;

/*
 * 统一封装的提示框：全程序的确认/错误提示文案与样式在此集中定义。
 * createXxx() 只负责构造对话框（不进入事件循环），便于测试与自动截图；
 * confirmXxx()/showXxx() 在此基础上 exec() 并返回用户的选择。
 */
namespace MsgBox {

QMessageBox *createDeleteConfirm(QWidget *parent, const QString &what);
QMessageBox *createConfirm(QWidget *parent, const QString &title, const QString &text);
QMessageBox *createInfo(QWidget *parent, const QString &text);
QMessageBox *createWarning(QWidget *parent, const QString &text);
QMessageBox *createError(QWidget *parent, const QString &title, const QString &text);

bool confirmDelete(QWidget *parent, const QString &what);
bool confirm(QWidget *parent, const QString &title, const QString &text);

void showInfo(QWidget *parent, const QString &text);
void showWarning(QWidget *parent, const QString &text);
void showError(QWidget *parent, const QString &title, const QString &text);

} // namespace MsgBox

#endif // MESSAGEBOX_H
