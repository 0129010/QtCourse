#include "historypage.h"

#include "db/databasemanager.h"
#include "util/messagebox.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QTableView>
#include <QVBoxLayout>

HistoryPage::HistoryPage(QWidget *parent)
    : QWidget(parent)
    , m_tableView(nullptr)
    , m_model(nullptr)
    , m_statusLabel(nullptr)
{
    initUi();
    refresh();
}

void HistoryPage::initUi()
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(12, 10, 12, 8);
    root->setSpacing(8);

    QLabel *title = new QLabel(QStringLiteral("用户操作记录"));
    QFont titleFont = title->font();
    titleFont.setPointSize(13);
    titleFont.setBold(true);
    title->setFont(titleFont);
    root->addWidget(title);

    QHBoxLayout *bar = new QHBoxLayout;
    QPushButton *refreshButton = new QPushButton(QStringLiteral("刷新记录"), this);
    refreshButton->setObjectName(QStringLiteral("refreshHistoryButton"));
    refreshButton->setMinimumSize(94, 30);
    bar->addWidget(refreshButton);
    bar->addStretch();
    root->addLayout(bar);

    m_tableView = new QTableView(this);
    m_tableView->setObjectName(QStringLiteral("historyTable"));
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    root->addWidget(m_tableView, 1);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet(QStringLiteral("color:#555555;"));
    root->addWidget(m_statusLabel);

    connect(refreshButton, &QPushButton::clicked, this, &HistoryPage::refresh);
}

void HistoryPage::refresh()
{
    if (!m_model) {
        m_model = new QSqlQueryModel(this);
    }
    m_model->setQuery(QStringLiteral(
        "SELECT h.TIMESTAMP AS \"时间\", u.FULLNAME AS \"操作人\", h.USER_ID AS \"用户ID\", "
        "h.EVENT AS \"操作内容\" FROM history h LEFT JOIN user u ON u.ID = h.USER_ID "
        "ORDER BY h.TIMESTAMP DESC, h.rowid DESC"),
        DatabaseManager::instance().database());

    if (m_model->lastError().isValid()) {
        MsgBox::showError(this, QStringLiteral("读取操作记录失败"), m_model->lastError().text());
        return;
    }

    if (m_tableView->model() != m_model) {
        m_tableView->setModel(m_model);
    }
    m_statusLabel->setText(QStringLiteral("共 %1 条操作记录（按时间倒序）").arg(m_model->rowCount()));
}
