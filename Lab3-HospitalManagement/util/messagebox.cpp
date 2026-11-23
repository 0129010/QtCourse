#include "messagebox.h"

#include <QAbstractButton>
#include <QPushButton>

namespace {

// 统一给对话框按钮设置中文文案，并通过标准角色判断点击结果
void addButtons(QMessageBox *box, const QString &acceptText, const QString &rejectText)
{
    box->addButton(acceptText, QMessageBox::AcceptRole);
    if (!rejectText.isEmpty()) {
        box->addButton(rejectText, QMessageBox::RejectRole);
    }
}

bool execAndCheckAccepted(QMessageBox *box)
{
    box->exec();
    const bool accepted = box->clickedButton()
                          && box->buttonRole(box->clickedButton()) == QMessageBox::AcceptRole;
    delete box;
    return accepted;
}

} // namespace

namespace MsgBox {

QMessageBox *createDeleteConfirm(QWidget *parent, const QString &what)
{
    QMessageBox *box = new QMessageBox(parent);
    box->setWindowTitle(QStringLiteral("删除确认"));
    box->setIcon(QMessageBox::Question);
    box->setText(QStringLiteral("确定要删除%1吗？").arg(what));
    box->setInformativeText(QStringLiteral("删除后该记录将从数据库中移除，操作不可撤销。"));
    addButtons(box, QStringLiteral("删除"), QStringLiteral("取消"));
    box->setDefaultButton(qobject_cast<QPushButton *>(box->buttons().at(1)));
    return box;
}

QMessageBox *createConfirm(QWidget *parent, const QString &title, const QString &text)
{
    QMessageBox *box = new QMessageBox(parent);
    box->setWindowTitle(title);
    box->setIcon(QMessageBox::Question);
    box->setText(text);
    addButtons(box, QStringLiteral("确定"), QStringLiteral("取消"));
    box->setDefaultButton(qobject_cast<QPushButton *>(box->buttons().at(1)));
    return box;
}

QMessageBox *createInfo(QWidget *parent, const QString &text)
{
    QMessageBox *box = new QMessageBox(parent);
    box->setWindowTitle(QStringLiteral("提示"));
    box->setIcon(QMessageBox::Information);
    box->setText(text);
    addButtons(box, QStringLiteral("确定"), QString());
    return box;
}

QMessageBox *createWarning(QWidget *parent, const QString &text)
{
    QMessageBox *box = new QMessageBox(parent);
    box->setWindowTitle(QStringLiteral("输入校验"));
    box->setIcon(QMessageBox::Warning);
    box->setText(text);
    addButtons(box, QStringLiteral("确定"), QString());
    return box;
}

QMessageBox *createError(QWidget *parent, const QString &title, const QString &text)
{
    QMessageBox *box = new QMessageBox(parent);
    box->setWindowTitle(title);
    box->setIcon(QMessageBox::Critical);
    box->setText(text);
    addButtons(box, QStringLiteral("确定"), QString());
    return box;
}

bool confirmDelete(QWidget *parent, const QString &what)
{
    return execAndCheckAccepted(createDeleteConfirm(parent, what));
}

bool confirm(QWidget *parent, const QString &title, const QString &text)
{
    return execAndCheckAccepted(createConfirm(parent, title, text));
}

void showInfo(QWidget *parent, const QString &text)
{
    execAndCheckAccepted(createInfo(parent, text));
}

void showWarning(QWidget *parent, const QString &text)
{
    execAndCheckAccepted(createWarning(parent, text));
}

void showError(QWidget *parent, const QString &title, const QString &text)
{
    execAndCheckAccepted(createError(parent, title, text));
}

} // namespace MsgBox
