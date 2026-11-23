#include "patientdialog.h"

#include "db/databasemanager.h"
#include "util/messagebox.h"

#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>
#include <QPushButton>
#include <QVBoxLayout>

PatientDialog::PatientDialog(const QVariantMap &initial, bool editMode, QWidget *parent)
    : QDialog(parent)
    , m_editMode(editMode)
    , m_idEdit(nullptr)
    , m_nameEdit(nullptr)
    , m_idCardEdit(nullptr)
    , m_sexCombo(nullptr)
    , m_dobEdit(nullptr)
    , m_heightSpin(nullptr)
    , m_weightSpin(nullptr)
    , m_phoneEdit(nullptr)
{
    initUi(initial);
}

void PatientDialog::initUi(const QVariantMap &initial)
{
    setWindowTitle(m_editMode ? QStringLiteral("修改患者信息") : QStringLiteral("添加患者信息"));
    setFixedWidth(420);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 16, 20, 16);
    root->setSpacing(10);

    QLabel *hint = new QLabel(m_editMode
                                  ? QStringLiteral("带 * 的字段为必填项，ID 为主键不可修改。")
                                  : QStringLiteral("带 * 的字段为必填项，ID 由系统自动生成或手工填写。"), this);
    hint->setStyleSheet(QStringLiteral("color:#666666;"));
    hint->setWordWrap(true);
    root->addWidget(hint);

    QFormLayout *form = new QFormLayout;
    form->setLabelAlignment(Qt::AlignRight);
    form->setSpacing(8);

    m_idEdit = new QLineEdit(this);
    m_idEdit->setObjectName(QStringLiteral("idEdit"));
    m_idEdit->setPlaceholderText(QStringLiteral("如 P0009"));
    if (m_editMode) {
        m_idEdit->setEnabled(false);
    }

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setObjectName(QStringLiteral("nameEdit"));

    m_idCardEdit = new QLineEdit(this);
    m_idCardEdit->setObjectName(QStringLiteral("idCardEdit"));
    m_idCardEdit->setPlaceholderText(QStringLiteral("18 位身份证号"));

    m_sexCombo = new QComboBox(this);
    m_sexCombo->setObjectName(QStringLiteral("sexCombo"));
    m_sexCombo->addItem(QStringLiteral("男"), 1);
    m_sexCombo->addItem(QStringLiteral("女"), 0);

    m_dobEdit = new QDateEdit(this);
    m_dobEdit->setObjectName(QStringLiteral("dobEdit"));
    m_dobEdit->setCalendarPopup(true);
    m_dobEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    m_dobEdit->setDate(QDate(2000, 1, 1));

    m_heightSpin = new QDoubleSpinBox(this);
    m_heightSpin->setObjectName(QStringLiteral("heightSpin"));
    m_heightSpin->setRange(50.0, 250.0);
    m_heightSpin->setDecimals(1);
    m_heightSpin->setSingleStep(0.5);
    m_heightSpin->setSuffix(QStringLiteral(" cm"));

    m_weightSpin = new QDoubleSpinBox(this);
    m_weightSpin->setObjectName(QStringLiteral("weightSpin"));
    m_weightSpin->setRange(10.0, 300.0);
    m_weightSpin->setDecimals(1);
    m_weightSpin->setSingleStep(0.5);
    m_weightSpin->setSuffix(QStringLiteral(" kg"));

    m_phoneEdit = new QLineEdit(this);
    m_phoneEdit->setObjectName(QStringLiteral("phoneEdit"));
    m_phoneEdit->setPlaceholderText(QStringLiteral("11 位手机号"));

    form->addRow(QStringLiteral("* ID："), m_idEdit);
    form->addRow(QStringLiteral("* 姓名："), m_nameEdit);
    form->addRow(QStringLiteral("  身份证："), m_idCardEdit);
    form->addRow(QStringLiteral("  性别："), m_sexCombo);
    form->addRow(QStringLiteral("  出生日期："), m_dobEdit);
    form->addRow(QStringLiteral("  身高："), m_heightSpin);
    form->addRow(QStringLiteral("  体重："), m_weightSpin);
    form->addRow(QStringLiteral("  手机号："), m_phoneEdit);
    root->addLayout(form);

    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addStretch();
    QPushButton *saveButton = new QPushButton(QStringLiteral("保存"), this);
    saveButton->setObjectName(QStringLiteral("saveButton"));
    saveButton->setMinimumSize(88, 30);
    QPushButton *cancelButton = new QPushButton(QStringLiteral("取消"), this);
    cancelButton->setObjectName(QStringLiteral("cancelButton"));
    cancelButton->setMinimumSize(88, 30);
    buttons->addWidget(saveButton);
    buttons->addSpacing(10);
    buttons->addWidget(cancelButton);
    root->addLayout(buttons);

    connect(saveButton, &QPushButton::clicked, this, &PatientDialog::onAcceptClicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // 回填已有数据
    if (!initial.isEmpty()) {
        m_idEdit->setText(initial.value(QStringLiteral("ID")).toString());
        m_nameEdit->setText(initial.value(QStringLiteral("NAME")).toString());
        m_idCardEdit->setText(initial.value(QStringLiteral("ID_CARD")).toString());
        m_sexCombo->setCurrentIndex(m_sexCombo->findData(initial.value(QStringLiteral("SEX")).toInt()));
        const QDate dob = QDate::fromString(initial.value(QStringLiteral("DOB")).toString(),
                                            QStringLiteral("yyyy-MM-dd"));
        if (dob.isValid()) {
            m_dobEdit->setDate(dob);
        }
        m_heightSpin->setValue(initial.value(QStringLiteral("HEIGHT")).toDouble());
        m_weightSpin->setValue(initial.value(QStringLiteral("WEIGHT")).toDouble());
        m_phoneEdit->setText(initial.value(QStringLiteral("MOBILEPHONE")).toString());
    } else {
        m_idEdit->setText(DatabaseManager::instance().nextId(QStringLiteral("patient"),
                                                             QStringLiteral("P")));
    }
}

QString PatientDialog::patientId() const
{
    return m_idEdit->text().trimmed();
}

QVariantMap PatientDialog::patientData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("ID"), m_idEdit->text().trimmed());
    data.insert(QStringLiteral("NAME"), m_nameEdit->text().trimmed());
    data.insert(QStringLiteral("ID_CARD"), m_idCardEdit->text().trimmed());
    data.insert(QStringLiteral("SEX"), m_sexCombo->currentData().toInt());
    data.insert(QStringLiteral("DOB"), m_dobEdit->date().toString(QStringLiteral("yyyy-MM-dd")));
    data.insert(QStringLiteral("HEIGHT"), m_heightSpin->value());
    data.insert(QStringLiteral("WEIGHT"), m_weightSpin->value());
    data.insert(QStringLiteral("MOBILEPHONE"), m_phoneEdit->text().trimmed());
    return data;
}

bool PatientDialog::idExists(const QString &id) const
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(QStringLiteral("SELECT COUNT(*) FROM patient WHERE ID = ?"));
    query.addBindValue(id);
    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

bool PatientDialog::validateInput(QString *message) const
{
    const QString id = m_idEdit->text().trimmed();
    const QString name = m_nameEdit->text().trimmed();
    const QString idCard = m_idCardEdit->text().trimmed();
    const QString phone = m_phoneEdit->text().trimmed();

    if (id.isEmpty()) {
        *message = QStringLiteral("ID 不能为空。");
        return false;
    }
    if (name.isEmpty()) {
        *message = QStringLiteral("姓名不能为空，请填写患者真实姓名。");
        return false;
    }
    if (!m_editMode && idExists(id)) {
        *message = QStringLiteral("ID“%1”已存在，请更换主键后再保存。").arg(id);
        return false;
    }
    if (!idCard.isEmpty()) {
        QRegularExpression re(QStringLiteral("^[0-9]{17}[0-9Xx]$"));
        if (!re.match(idCard).hasMatch()) {
            *message = QStringLiteral("身份证号格式不正确，应为 18 位（末位可为 X）。");
            return false;
        }
    }
    if (!phone.isEmpty()) {
        QRegularExpression re(QStringLiteral("^1[0-9]{10}$"));
        if (!re.match(phone).hasMatch()) {
            *message = QStringLiteral("手机号格式不正确，应为以 1 开头的 11 位数字。");
            return false;
        }
    }
    if (m_heightSpin->value() <= 0 || m_weightSpin->value() <= 0) {
        *message = QStringLiteral("身高、体重必须大于 0。");
        return false;
    }
    return true;
}

void PatientDialog::onAcceptClicked()
{
    QString message;
    if (!validateInput(&message)) {
        MsgBox::showWarning(this, message);
        return;
    }
    accept();
}
