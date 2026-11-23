#include "departmentpage.h"

#include "db/databasemanager.h"
#include "util/messagebox.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QTableView>
#include <QVBoxLayout>

DepartmentPage::DepartmentPage(const QString &userId, QWidget *parent)
    : QWidget(parent)
    , m_userId(userId)
    , m_tableView(nullptr)
    , m_model(nullptr)
    , m_statusLabel(nullptr)
{
    initUi();
    setupModel();
    refresh();
}

void DepartmentPage::initUi()
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(12, 10, 12, 8);
    root->setSpacing(8);

    QLabel *title = new QLabel(QStringLiteral("科室信息管理"));
    QFont titleFont = title->font();
    titleFont.setPointSize(13);
    titleFont.setBold(true);
    title->setFont(titleFont);
    root->addWidget(title);

    QHBoxLayout *bar = new QHBoxLayout;
    QPushButton *addButton = new QPushButton(QStringLiteral("添加科室"), this);
    addButton->setObjectName(QStringLiteral("addDepartmentButton"));
    QPushButton *deleteButton = new QPushButton(QStringLiteral("删除科室"), this);
    deleteButton->setObjectName(QStringLiteral("deleteDepartmentButton"));
    QPushButton *refreshButton = new QPushButton(QStringLiteral("刷新"), this);
    refreshButton->setObjectName(QStringLiteral("refreshDepartmentButton"));
    for (QPushButton *button : { addButton, deleteButton, refreshButton }) {
        button->setMinimumSize(90, 30);
    }
    bar->addWidget(addButton);
    bar->addWidget(deleteButton);
    bar->addWidget(refreshButton);
    bar->addStretch();
    root->addLayout(bar);

    m_tableView = new QTableView(this);
    m_tableView->setObjectName(QStringLiteral("departmentTable"));
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    root->addWidget(m_tableView, 1);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet(QStringLiteral("color:#555555;"));
    root->addWidget(m_statusLabel);

    connect(addButton, &QPushButton::clicked, this, &DepartmentPage::onAddClicked);
    connect(deleteButton, &QPushButton::clicked, this, &DepartmentPage::onDeleteClicked);
    connect(refreshButton, &QPushButton::clicked, this, &DepartmentPage::refresh);
}

void DepartmentPage::setupModel()
{
    m_model = new QSqlTableModel(this, DatabaseManager::instance().database());
    m_model->setTable(QStringLiteral("department"));
    m_model->setEditStrategy(QSqlTableModel::OnRowChange);
    m_model->setHeaderData(0, Qt::Horizontal, QStringLiteral("科室编号"));
    m_model->setHeaderData(1, Qt::Horizontal, QStringLiteral("科室名称"));
    m_tableView->setModel(m_model);
}

void DepartmentPage::refresh()
{
    m_model->setFilter(QString());
    m_model->select();
    if (m_model->lastError().isValid()) {
        MsgBox::showError(this, QStringLiteral("读取科室数据失败"), m_model->lastError().text());
    }
    m_statusLabel->setText(QStringLiteral("当前共 %1 个科室").arg(m_model->rowCount()));
}

void DepartmentPage::onAddClicked()
{
    bool ok = false;
    const QString name = QInputDialog::getText(this, QStringLiteral("添加科室"),
                                               QStringLiteral("科室名称："), QLineEdit::Normal,
                                               QString(), &ok).trimmed();
    if (!ok) {
        return;
    }
    if (name.isEmpty()) {
        MsgBox::showWarning(this, QStringLiteral("科室名称不能为空。"));
        return;
    }

    const QString id = DatabaseManager::instance().nextId(QStringLiteral("department"),
                                                          QStringLiteral("DP"), 2);
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(QStringLiteral("INSERT INTO department (ID, NAME) VALUES (?, ?)"));
    query.addBindValue(id);
    query.addBindValue(name);
    if (!query.exec()) {
        MsgBox::showError(this, QStringLiteral("添加科室失败"), query.lastError().text());
        return;
    }

    DatabaseManager::instance().logEvent(m_userId, QStringLiteral("添加科室 %1（%2）").arg(name, id));
    refresh();
}

void DepartmentPage::onDeleteClicked()
{
    const QModelIndex index = m_tableView->currentIndex();
    if (!index.isValid()) {
        MsgBox::showWarning(this, QStringLiteral("请先选择要删除的科室。"));
        return;
    }
    const QSqlRecord record = m_model->record(index.row());
    const QString id = record.value(QStringLiteral("ID")).toString();
    const QString name = record.value(QStringLiteral("NAME")).toString();

    // 外键检查：仍有医生属于该科室时禁止删除
    QSqlQuery count(DatabaseManager::instance().database());
    count.prepare(QStringLiteral("SELECT COUNT(*) FROM doctor WHERE DEPARTMENT_ID = ?"));
    count.addBindValue(id);
    int doctors = 0;
    if (count.exec() && count.next()) {
        doctors = count.value(0).toInt();
    }
    if (doctors > 0) {
        MsgBox::showWarning(this, QStringLiteral("科室“%1”下还有 %2 名医生，请先调整医生的科室后再删除。")
                                   .arg(name).arg(doctors));
        return;
    }

    if (!MsgBox::confirmDelete(this, QStringLiteral("科室 %1（%2）").arg(name, id))) {
        return;
    }

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(QStringLiteral("DELETE FROM department WHERE ID = ?"));
    query.addBindValue(id);
    if (!query.exec()) {
        MsgBox::showError(this, QStringLiteral("删除科室失败"), query.lastError().text());
        return;
    }

    DatabaseManager::instance().logEvent(m_userId, QStringLiteral("删除科室 %1（%2）").arg(name, id));
    refresh();
}
