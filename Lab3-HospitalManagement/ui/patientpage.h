#ifndef PATIENTPAGE_H
#define PATIENTPAGE_H

#include <QWidget>
#include <QString>
#include <QVariantMap>

class QLineEdit;
class QPushButton;
class QTableView;
class QSqlTableModel;
class QLabel;

/*
 * 患者管理页（Controller + View）：
 *  - View：搜索框、工具栏按钮、QTableView
 *  - Model：QSqlTableModel（绑定 patient 表）+ 自定义代理（性别下拉、身高体重数值框）
 *  - 查询：QSqlQuery 使用绑定参数做通用查询，再把命中的主键集合交给 Model 过滤显示
 */
class PatientPage : public QWidget
{
    Q_OBJECT
public:
    explicit PatientPage(const QString &userId, QWidget *parent = nullptr);

public slots:
    void refresh();
    void onSearchClicked();
    void onAddClicked();
    void onModifyClicked();
    void onDeleteClicked();
    void setSearchKeyword(const QString &keyword);

private:
    void initUi();
    void setupModel();
    void updateStatus(const QString &extra = QString());
    QString currentPatientId() const;
    QVariantMap currentPatientData() const;
    int birthdayToAge(const QString &dob) const;
    bool runPatientInsert(const QVariantMap &data, QString *errorText);
    bool runPatientUpdate(const QVariantMap &data, QString *errorText);
    void clearSearch();

    QString m_userId;

    QLineEdit *m_searchEdit;
    QPushButton *m_searchButton;
    QPushButton *m_addButton;
    QPushButton *m_modifyButton;
    QPushButton *m_deleteButton;
    QPushButton *m_refreshButton;
    QTableView *m_tableView;
    QSqlTableModel *m_model;
    QLabel *m_statusLabel;
};

#endif // PATIENTPAGE_H
