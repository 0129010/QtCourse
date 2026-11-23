#include "logindialog.h"

#include "db/databasemanager.h"

#include <QFont>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
{
    initUi();
}

void LoginDialog::initUi()
{
    setWindowTitle(QStringLiteral("医院诊疗信息管理系统 - 登录"));
    setFixedSize(460, 320);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(48, 28, 48, 24);
    root->setSpacing(14);

    QLabel *title = new QLabel(QStringLiteral("欢迎使用医院诊疗信息管理系统"), this);
    QFont titleFont = title->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);
    root->addWidget(title);

    QLabel *subtitle = new QLabel(QStringLiteral("请使用系统分配的账号登录"), this);
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet(QStringLiteral("color:#666666;"));
    root->addWidget(subtitle);

    QFormLayout *form = new QFormLayout;
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);

    m_accountEdit = new QLineEdit(this);
    m_accountEdit->setObjectName(QStringLiteral("accountEdit"));
    m_accountEdit->setPlaceholderText(QStringLiteral("请输入用户名，如 admin"));
    m_accountEdit->setMinimumHeight(28);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setObjectName(QStringLiteral("passwordEdit"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(QStringLiteral("请输入密码"));
    m_passwordEdit->setMinimumHeight(28);

    form->addRow(QStringLiteral("用户名："), m_accountEdit);
    form->addRow(QStringLiteral("密    码："), m_passwordEdit);
    root->addLayout(form);

    m_errorLabel = new QLabel(this);
    m_errorLabel->setObjectName(QStringLiteral("errorLabel"));
    m_errorLabel->setStyleSheet(QStringLiteral("color:#c0392b;"));
    m_errorLabel->setWordWrap(true);
    m_errorLabel->setMinimumHeight(36);
    root->addWidget(m_errorLabel);

    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addStretch();
    m_loginButton = new QPushButton(QStringLiteral("登 录"), this);
    m_loginButton->setObjectName(QStringLiteral("loginButton"));
    m_loginButton->setMinimumSize(96, 32);
    m_loginButton->setDefault(true);
    m_quitButton = new QPushButton(QStringLiteral("退 出"), this);
    m_quitButton->setObjectName(QStringLiteral("quitButton"));
    m_quitButton->setMinimumSize(96, 32);
    buttons->addWidget(m_loginButton);
    buttons->addSpacing(12);
    buttons->addWidget(m_quitButton);
    root->addLayout(buttons);

    connect(m_loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(m_quitButton, &QPushButton::clicked, this, &LoginDialog::reject);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLoginClicked);

    m_accountEdit->setFocus();
    m_errorLabel->clear();
}

void LoginDialog::showErrorText(const QString &text)
{
    m_errorLabel->setText(text);
}

QString LoginDialog::errorMessage() const
{
    return m_errorLabel->text();
}

void LoginDialog::setCredentials(const QString &account, const QString &password)
{
    m_accountEdit->setText(account);
    m_passwordEdit->setText(password);
}

bool LoginDialog::submit()
{
    onLoginClicked();
    return !m_userId.isEmpty();
}

void LoginDialog::onLoginClicked()
{
    const QString account = m_accountEdit->text().trimmed();
    const QString password = m_passwordEdit->text();

    if (account.isEmpty() || password.isEmpty()) {
        showErrorText(QStringLiteral("用户名和密码不能为空。"));
        return;
    }

    QString fullName;
    QString userId;
    if (!DatabaseManager::instance().login(account, password, &fullName, &userId)) {
        showErrorText(QStringLiteral("登录失败：用户名或密码错误，请重新输入。"));
        m_passwordEdit->clear();
        m_passwordEdit->setFocus();
        return;
    }

    m_userId = userId;
    m_userFullName = fullName;
    DatabaseManager::instance().logEvent(userId, QStringLiteral("登录系统"));
    accept();
}
