#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QString>

class QLineEdit;
class QLabel;
class QPushButton;

/*
 * 登录界面（View）：只负责收集用户名/密码并给出提示，
 * 真正的校验交给 DatabaseManager::login()（Model），属于典型的 View → Model 调用。
 */
class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);

    QString userId() const { return m_userId; }
    QString userFullName() const { return m_userFullName; }

    // 供自动化截图使用：填入账号密码并触发一次登录
    void setCredentials(const QString &account, const QString &password);
    bool submit();

    QString errorMessage() const;

private slots:
    void onLoginClicked();

private:
    void initUi();
    void showErrorText(const QString &text);

    QLineEdit *m_accountEdit;
    QLineEdit *m_passwordEdit;
    QLabel *m_errorLabel;
    QPushButton *m_loginButton;
    QPushButton *m_quitButton;

    QString m_userId;
    QString m_userFullName;
};

#endif // LOGINDIALOG_H
