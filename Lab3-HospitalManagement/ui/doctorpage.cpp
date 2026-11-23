#include "doctorpage.h"

#include "db/databasemanager.h"
#include "ui/delegate/comboboxdelegate.h"
#include "util/messagebox.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QTableView>
#include <QVBoxLayout>

DoctorPage::DoctorPage(const QString &userId, QWidget *parent)
    : QWidget(parent)
    , m_userId(userId)
    , m_tableView(nullptr)
    , m_model(nullptr)
    , m_statusLabel(nullptr)
    , m_addButton(nullptr)
    , m_deleteButton(nullptr)
    , m_refreshButton(nullptr)
{
    initUi();
    setupModel();
    refresh();
}

void DoctorPage::initUi()
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(12, 10, 12, 8);
    root->setSpacing(8);

    QLabel *title = new QLabel(QStringLiteral("医生信息管理"));
    QFont titleFont = title->font();
    titleFont.setPointSize(13);
    titleFont.setBold(true);
    title->setFont(titleFont);
    root->addWidget(title);

    QLabel *tip = new QLabel(QStringLiteral("提示：双击“科室”单元格可从下拉框中选择所属科室，双击其他单元格可直接修改文字。"));
    tip->setStyleSheet(QStringLiteral("color:#666666;"));
    root->addWidget(tip);

    QHBoxLayout *bar = new QHBoxLayout;
    m_addButton = new QPushButton(QStringLiteral("添加医生"), this);
    m_addButton->setObjectName(QStringLiteral("addDoctorButton"));
    m_deleteButton = new QPushButton(QStringLiteral("删除医生"), this);
    m_deleteButton->setObjectName(QStringLiteral("deleteDoctorButton"));
    m_refreshButton = new QPushButton(QStringLiteral("刷新"), this);
    m_refreshButton->setObjectName(QStringLiteral("refreshDoctorButton"));
    for (QPushButton *button : { m_addButton, m_deleteButton, m_refreshButton }) {
        button->setMinimumSize(90, 30);
    }
    bar->addWidget(m_addButton);
    bar->addWidget(m_deleteButton);
    bar->addWidget(m_refreshButton);
    bar->addStretch();
    root->addLayout(bar);

    m_tableView = new QTableView(this);
    m_tableView->setObjectName(QStringLiteral("doctorTable"));
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    root->addWidget(m_tableView, 1);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet(QStringLiteral("color:#555555;"));
    root->addWidget(m_statusLabel);

    connect(m_addButton, &QPushButton::clicked, this, &DoctorPage::onAddClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &DoctorPage::onDeleteClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &DoctorPage::refresh);
}

void DoctorPage::setupModel()
{
    m_model = new QSqlTableModel(this, DatabaseManager::instance().database());
    m_model->setTable(QStringLiteral("doctor"));
    m_model->setEditStrategy(QSqlTableModel::OnRowChange);
    m_model->setHeaderData(0, Qt::Horizontal, QStringLiteral("医生ID"));
    m_model->setHeaderData(1, Qt::Horizontal, QStringLiteral("工号"));
    m_model->setHeaderData(2, Qt::Horizontal, QStringLiteral("姓名"));
    m_model->setHeaderData(3, Qt::Horizontal, QStringLiteral("科室"));

    // 科室列：下拉框代理，标签显示科室名、实际写入外键 DEPARTMENT_ID
    ComboBoxDelegate *deptDelegate = new ComboBoxDelegate(this);
    QStringList labels;
    QStringList values;
    QSqlQuery query(DatabaseManager::instance().database());
    if (query.exec(QStringLiteral("SELECT ID, NAME FROM department ORDER BY ID"))) {
        while (query.next()) {
            values << query.value(0).toString();
            labels << query.value(1).toString();
        }
    }
    deptDelegate->setItems(labels, values);
    m_tableView->setItemDelegateForColumn(3, deptDelegate);

    m_tableView->setModel(m_model);
}

void DoctorPage::refresh()
{
    m_model->setFilter(QString());
    m_model->select();
    if (m_model->lastError().isValid()) {
        MsgBox::showError(this, QStringLiteral("读取医生数据失败"), m_model->lastError().text());
    }
    updateStatus();
}

void DoctorPage::updateStatus(const QString &extra)
{
    const int rows = m_model ? m_model->rowCount() : 0;
    m_statusLabel->setText(extra.isEmpty()
                               ? QStringLiteral("当前共 %1 名医生").arg(rows)
                               : extra);
}

void DoctorPage::onAddClicked()
{
    const QString id = DatabaseManager::instance().nextId(QStringLiteral("doctor"),
                                                          QStringLiteral("D"));
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(QStringLiteral("INSERT INTO doctor (ID, EMPLOYEE_NO, NAME, DEPARTMENT_ID) VALUES (?, ?, ?, ?)"));
    query.addBindValue(id);
    query.addBindValue(QStringLiteral("E2026%1").arg(id.right(3)));
    query.addBindValue(QStringLiteral("新医生"));
    query.addBindValue(QStringLiteral("DP01"));
    if (!query.exec()) {
        MsgBox::showError(this, QStringLiteral("添加医生失败"), query.lastError().text());
        return;
    }

    DatabaseManager::instance().logEvent(m_userId, QStringLiteral("添加医生 %1").arg(id));
    refresh();
    updateStatus(QStringLiteral("已添加医生 %1，可在表格中双击修改姓名与科室").arg(id));
}

void DoctorPage::onDeleteClicked()
{
    const QModelIndex index = m_tableView->currentIndex();
    if (!index.isValid()) {
        MsgBox::showWarning(this, QStringLiteral("请先选择要删除的医生记录。"));
        return;
    }
    const QSqlRecord record = m_model->record(index.row());
    const QString id = record.value(QStringLiteral("ID")).toString();
    const QString name = record.value(QStringLiteral("NAME")).toString();

    if (!MsgBox::confirmDelete(this, QStringLiteral("医生 %1（%2）").arg(name, id))) {
        return;
    }

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(QStringLiteral("DELETE FROM doctor WHERE ID = ?"));
    query.addBindValue(id);
    if (!query.exec()) {
        MsgBox::showError(this, QStringLiteral("删除医生失败"), query.lastError().text());
        return;
    }

    DatabaseManager::instance().logEvent(m_userId, QStringLiteral("删除医生 %1（%2）").arg(name, id));
    refresh();
    updateStatus(QStringLiteral("已删除医生 %1").arg(name));
}
