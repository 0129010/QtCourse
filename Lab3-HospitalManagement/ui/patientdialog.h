#ifndef PATIENTDIALOG_H
#define PATIENTDIALOG_H

#include <QDialog>
#include <QString>
#include <QVariantMap>

class QLineEdit;
class QComboBox;
class QDateEdit;
class QDoubleSpinBox;

/*
 * 患者信息的添加/修改对话框。
 * 负责界面的输入与字段校验（必填项、身份证/手机号格式、主键重复检查），
 * 真正的数据库写入由 PatientPage 完成。
 */
class PatientDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PatientDialog(const QVariantMap &initial, bool editMode,
                           QWidget *parent = nullptr);

    QVariantMap patientData() const;
    QString patientId() const;

private slots:
    void onAcceptClicked();

private:
    void initUi(const QVariantMap &initial);
    bool validateInput(QString *message) const;
    bool idExists(const QString &id) const;

    bool m_editMode;
    QLineEdit *m_idEdit;
    QLineEdit *m_nameEdit;
    QLineEdit *m_idCardEdit;
    QComboBox *m_sexCombo;
    QDateEdit *m_dobEdit;
    QDoubleSpinBox *m_heightSpin;
    QDoubleSpinBox *m_weightSpin;
    QLineEdit *m_phoneEdit;
};

#endif // PATIENTDIALOG_H
