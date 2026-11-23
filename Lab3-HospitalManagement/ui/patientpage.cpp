#include "patientpage.h"

#include "db/databasemanager.h"
#include "db/patienttablemodel.h"
#include "ui/delegate/comboboxdelegate.h"
#include "ui/delegate/doublespindelegate.h"
#include "ui/patientdialog.h"
#include "util/messagebox.h"

#include <QDate>
#include <QDateTime>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QTableView>
#include <QVBoxLayout>
#include <QVariantMap>

PatientPage::PatientPage(const QString &userId, QWidget *parent)
    : QWidget(parent)
    , m_userId(userId)
    , m_searchEdit(nullptr)
    , m_searchButton(nullptr)
    , m_addButton(nullptr)
    , m_modifyButton(nullptr)
    , m_deleteButton(nullptr)
    , m_refreshButton(nullptr)
    , m_tableView(nullptr)
    , m_model(nullptr)
    , m_statusLabel(nullptr)
{
    initUi();
    setupModel();
    refresh();
}

void PatientPage::initUi()
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(12, 10, 12, 8);
    root->setSpacing(8);

    QLabel *title = new QLabel(QStringLiteral("患者信息管理"));
    QFont titleFont = title->font();
    titleFont.setPointSize(13);
    titleFont.setBold(true);
    title->setFont(titleFont);
    root->addWidget(title);

    QHBoxLayout *bar = new QHBoxLayout;
    bar->setSpacing(8);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setObjectName(QStringLiteral("searchEdit"));
    m_searchEdit->setPlaceholderText(QStringLiteral("输入姓名 / 身份证号 / 手机号进行查询"));
    m_searchEdit->setMinimumHeight(30);
    m_searchEdit->setClearButtonEnabled(true);

    m_searchButton = new QPushButton(QStringLiteral("查找"), this);
    m_searchButton->setObjectName(QStringLiteral("searchButton"));
    m_addButton = new QPushButton(QStringLiteral("添加"), this);
    m_addButton->setObjectName(QStringLiteral("addButton"));
    m_modifyButton = new QPushButton(QStringLiteral("修改"), this);
    m_modifyButton->setObjectName(QStringLiteral("modifyButton"));
    m_deleteButton = new QPushButton(QStringLiteral("删除"), this);
    m_deleteButton->setObjectName(QStringLiteral("deleteButton"));
    m_refreshButton = new QPushButton(QStringLiteral("刷新"), this);
    m_refreshButton->setObjectName(QStringLiteral("refreshButton"));

    const QList<QPushButton *> buttons = { m_searchButton, m_addButton, m_modifyButton,
                                            m_deleteButton, m_refreshButton };
    for (QPushButton *button : buttons) {
        button->setMinimumSize(74, 30);
    }

    bar->addWidget(m_searchEdit, 1);
    bar->addWidget(m_searchButton);
    bar->addWidget(m_addButton);
    bar->addWidget(m_modifyButton);
    bar->addWidget(m_deleteButton);
    bar->addWidget(m_refreshButton);
    root->addLayout(bar);

    m_tableView = new QTableView(this);
    m_tableView->setObjectName(QStringLiteral("patientTable"));
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setSortingEnabled(false);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    root->addWidget(m_tableView, 1);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName(QStringLiteral("statusLabel"));
    m_statusLabel->setStyleSheet(QStringLiteral("color:#555555;"));
    root->addWidget(m_statusLabel);

    connect(m_searchButton, &QPushButton::clicked, this, &PatientPage::onSearchClicked);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &PatientPage::onSearchClicked);
    connect(m_addButton, &QPushButton::clicked, this, &PatientPage::onAddClicked);
    connect(m_modifyButton, &QPushButton::clicked, this, &PatientPage::onModifyClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &PatientPage::onDeleteClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &PatientPage::refresh);
}

void PatientPage::setupModel()
{
    m_model = new PatientTableModel(this, DatabaseManager::instance().database());
    m_model->setTable(QStringLiteral("patient"));
    // 行内编辑：一行编辑结束后自动提交，保证界面与数据库一致
    m_model->setEditStrategy(QSqlTableModel::OnRowChange);
    m_model->setHeaderData(0, Qt::Horizontal, QStringLiteral("ID"));
    m_model->setHeaderData(1, Qt::Horizontal, QStringLiteral("身份证号"));
    m_model->setHeaderData(2, Qt::Horizontal, QStringLiteral("姓名"));
    m_model->setHeaderData(3, Qt::Horizontal, QStringLiteral("性别"));
    m_model->setHeaderData(4, Qt::Horizontal, QStringLiteral("出生日期"));
    m_model->setHeaderData(5, Qt::Horizontal, QStringLiteral("身高(cm)"));
    m_model->setHeaderData(6, Qt::Horizontal, QStringLiteral("体重(kg)"));
    m_model->setHeaderData(7, Qt::Horizontal, QStringLiteral("手机号"));
    m_model->setHeaderData(8, Qt::Horizontal, QStringLiteral("年龄"));
    m_model->setHeaderData(9, Qt::Horizontal, QStringLiteral("建档时间"));

    // 自定义代理：性别列用下拉框，身高/体重列用数值框
    // 注意代理挂在 View 上（QAbstractItemView::setItemDelegateForColumn），不是挂在 Model 上
    ComboBoxDelegate *sexDelegate = new ComboBoxDelegate(this);
    sexDelegate->setItems(QStringList() << QStringLiteral("男") << QStringLiteral("女"),
                          QStringList() << QStringLiteral("1") << QStringLiteral("0"));
    m_tableView->setItemDelegateForColumn(3, sexDelegate);
    m_tableView->setItemDelegateForColumn(5, new DoubleSpinDelegate(50.0, 250.0, 0.5, 1, this));
    m_tableView->setItemDelegateForColumn(6, new DoubleSpinDelegate(10.0, 300.0, 0.5, 1, this));

    m_tableView->setModel(m_model);

    // 表格内直接修改后写一条操作记录，便于追溯
    connect(m_model, &QSqlTableModel::dataChanged, this, [this](const QModelIndex &topLeft,
                                                                const QModelIndex &bottomRight) {
        Q_UNUSED(bottomRight)
        const QSqlRecord record = m_model->record(topLeft.row());
        DatabaseManager::instance().logEvent(
            m_userId, QStringLiteral("表格内修改患者 %1 的 %2 字段")
                          .arg(record.value(QStringLiteral("ID")).toString(),
                               m_model->headerData(topLeft.column(), Qt::Horizontal).toString()));
    });
}

void PatientPage::refresh()
{
    clearSearch();
    m_model->setFilter(QString());
    m_model->select();
    if (m_model->lastError().isValid()) {
        MsgBox::showError(this, QStringLiteral("读取数据失败"), m_model->lastError().text());
    }
    updateStatus();
}

void PatientPage::clearSearch()
{
    if (m_searchEdit) {
        m_searchEdit->clear();
    }
    if (m_model) {
        m_model->setFilter(QString());
    }
}

void PatientPage::updateStatus(const QString &extra)
{
    const int rows = m_model ? m_model->rowCount() : 0;
    QString text = QStringLiteral("当前显示 %1 条记录").arg(rows);
    if (!extra.isEmpty()) {
        text = extra;
    }
    m_statusLabel->setText(text);
}

void PatientPage::setSearchKeyword(const QString &keyword)
{
    m_searchEdit->setText(keyword);
}

void PatientPage::onSearchClicked()
{
    const QString keyword = m_searchEdit->text().trimmed();
    if (keyword.isEmpty()) {
        refresh();
        updateStatus(QStringLiteral("查询条件为空，已显示全部 %1 条记录").arg(m_model->rowCount()));
        return;
    }

    // 使用 QSqlQuery 的执行计划：绑定参数、避免手工拼接关键字
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(QStringLiteral("SELECT ID FROM patient "
                                 "WHERE NAME LIKE :kw OR ID_CARD LIKE :kw OR MOBILEPHONE LIKE :kw "
                                 "ORDER BY ID"));
    query.bindValue(QStringLiteral(":kw"), QStringLiteral("%1%").arg(keyword));
    if (!query.exec()) {
        MsgBox::showError(this, QStringLiteral("查询失败"), query.lastError().text());
        return;
    }

    QStringList ids;
    while (query.next()) {
        ids << query.value(0).toString();
    }

    if (ids.isEmpty()) {
        // 查询无结果：清空视图并给出明确提示，而不是让表格停留在上一次结果
        m_model->setFilter(QStringLiteral("ID = ''"));
        m_model->select();
        updateStatus(QStringLiteral("未找到与“%1”匹配的患者记录（共 0 条）").arg(keyword));
        DatabaseManager::instance().logEvent(
            m_userId, QStringLiteral("查询患者（关键字：%1，结果 0 条）").arg(keyword));
        return;
    }

    QStringList quoted;
    for (const QString &id : ids) {
        quoted << QStringLiteral("'%1'").arg(id);
    }
    m_model->setFilter(QStringLiteral("ID IN (%1)").arg(quoted.join(QStringLiteral(","))));
    m_model->select();
    updateStatus(QStringLiteral("关键字“%1”共匹配 %2 条记录").arg(keyword).arg(ids.size()));
    DatabaseManager::instance().logEvent(
        m_userId, QStringLiteral("查询患者（关键字：%1，结果 %2 条）").arg(keyword).arg(ids.size()));
}

QString PatientPage::currentPatientId() const
{
    const QModelIndex index = m_tableView->currentIndex();
    if (!index.isValid()) {
        return QString();
    }
    return m_model->record(index.row()).value(QStringLiteral("ID")).toString();
}

QVariantMap PatientPage::currentPatientData() const
{
    QVariantMap map;
    const QModelIndex index = m_tableView->currentIndex();
    if (!index.isValid()) {
        return map;
    }
    const QSqlRecord record = m_model->record(index.row());
    for (int i = 0; i < record.count(); ++i) {
        map.insert(record.fieldName(i), record.value(i));
    }
    return map;
}

int PatientPage::birthdayToAge(const QString &dob) const
{
    const QDate birthday = QDate::fromString(dob, QStringLiteral("yyyy-MM-dd"));
    if (!birthday.isValid()) {
        return 0;
    }
    int age = QDate::currentDate().year() - birthday.year();
    if (QDate::currentDate() < birthday.addYears(age)) {
        --age;
    }
    return age;
}

bool PatientPage::runPatientInsert(const QVariantMap &data, QString *errorText)
{
    // 参数按 SQL 中占位符的顺序绑定，避免位置错乱导致数据写错列
    QSqlQuery query(DatabaseManager::instance().database());
    const QString sql = QStringLiteral(
        "INSERT INTO patient (ID, ID_CARD, NAME, SEX, DOB, HEIGHT, WEIGHT, MOBILEPHONE, AGE, CREATEDTIMESTAMP) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    if (!query.prepare(sql)) {
        *errorText = query.lastError().text();
        return false;
    }
    query.addBindValue(data.value(QStringLiteral("ID")));
    query.addBindValue(data.value(QStringLiteral("ID_CARD")));
    query.addBindValue(data.value(QStringLiteral("NAME")));
    query.addBindValue(data.value(QStringLiteral("SEX")));
    query.addBindValue(data.value(QStringLiteral("DOB")));
    query.addBindValue(data.value(QStringLiteral("HEIGHT")));
    query.addBindValue(data.value(QStringLiteral("WEIGHT")));
    query.addBindValue(data.value(QStringLiteral("MOBILEPHONE")));
    query.addBindValue(birthdayToAge(data.value(QStringLiteral("DOB")).toString()));
    query.addBindValue(data.value(QStringLiteral("CREATEDTIMESTAMP")));
    if (!query.exec()) {
        *errorText = query.lastError().text();
        return false;
    }
    return true;
}

bool PatientPage::runPatientUpdate(const QVariantMap &data, QString *errorText)
{
    QSqlQuery query(DatabaseManager::instance().database());
    const QString sql = QStringLiteral(
        "UPDATE patient SET ID_CARD = ?, NAME = ?, SEX = ?, DOB = ?, HEIGHT = ?, "
        "WEIGHT = ?, MOBILEPHONE = ?, AGE = ? WHERE ID = ?");
    if (!query.prepare(sql)) {
        *errorText = query.lastError().text();
        return false;
    }
    query.addBindValue(data.value(QStringLiteral("ID_CARD")));
    query.addBindValue(data.value(QStringLiteral("NAME")));
    query.addBindValue(data.value(QStringLiteral("SEX")));
    query.addBindValue(data.value(QStringLiteral("DOB")));
    query.addBindValue(data.value(QStringLiteral("HEIGHT")));
    query.addBindValue(data.value(QStringLiteral("WEIGHT")));
    query.addBindValue(data.value(QStringLiteral("MOBILEPHONE")));
    query.addBindValue(birthdayToAge(data.value(QStringLiteral("DOB")).toString()));
    query.addBindValue(data.value(QStringLiteral("ID")));
    if (!query.exec()) {
        *errorText = query.lastError().text();
        return false;
    }
    return true;
}

void PatientPage::onAddClicked()
{
    PatientDialog dialog(QVariantMap(), false, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    QVariantMap data = dialog.patientData();
    data.insert(QStringLiteral("CREATEDTIMESTAMP"),
                QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));

    QString error;
    if (!runPatientInsert(data, &error)) {
        MsgBox::showError(this, QStringLiteral("添加失败"), error);
        return;
    }

    DatabaseManager::instance().logEvent(
        m_userId, QStringLiteral("添加患者 %1（%2）")
                     .arg(data.value(QStringLiteral("ID")).toString(),
                          data.value(QStringLiteral("NAME")).toString()));
    m_model->setFilter(QString());
    m_model->select();
    updateStatus(QStringLiteral("已添加患者 %1，列表已刷新（共 %2 条记录）")
                     .arg(data.value(QStringLiteral("NAME")).toString())
                     .arg(m_model->rowCount()));
    MsgBox::showInfo(this, QStringLiteral("患者 %1 添加成功，数据库与列表已同步更新。")
                              .arg(data.value(QStringLiteral("NAME")).toString()));
}

void PatientPage::onModifyClicked()
{
    const QString id = currentPatientId();
    if (id.isEmpty()) {
        MsgBox::showWarning(this, QStringLiteral("请先在表格中选择一条患者记录。"));
        return;
    }

    PatientDialog dialog(currentPatientData(), true, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QVariantMap data = dialog.patientData();
    QString error;
    if (!runPatientUpdate(data, &error)) {
        MsgBox::showError(this, QStringLiteral("修改失败"), error);
        return;
    }

    DatabaseManager::instance().logEvent(
        m_userId, QStringLiteral("修改患者 %1 的信息").arg(id));
    m_model->select();
    updateStatus(QStringLiteral("患者 %1 的信息已保存，列表已刷新").arg(id));
    MsgBox::showInfo(this, QStringLiteral("患者 %1 的信息修改成功。").arg(id));
}

void PatientPage::onDeleteClicked()
{
    const QString id = currentPatientId();
    if (id.isEmpty()) {
        MsgBox::showWarning(this, QStringLiteral("请先在表格中选择要删除的患者记录。"));
        return;
    }

    const QString name = currentPatientData().value(QStringLiteral("NAME")).toString();
    if (!MsgBox::confirmDelete(this, QStringLiteral("患者 %1（%2）").arg(name, id))) {
        return;
    }

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(QStringLiteral("DELETE FROM patient WHERE ID = ?"));
    query.addBindValue(id);
    if (!query.exec()) {
        MsgBox::showError(this, QStringLiteral("删除失败"), query.lastError().text());
        return;
    }

    DatabaseManager::instance().logEvent(m_userId, QStringLiteral("删除患者 %1（%2）").arg(name, id));
    m_model->select();
    updateStatus(QStringLiteral("已删除患者 %1，列表已刷新（剩余 %2 条记录）").arg(name).arg(m_model->rowCount()));
    MsgBox::showInfo(this, QStringLiteral("患者 %1 已从数据库中删除。").arg(name));
}
