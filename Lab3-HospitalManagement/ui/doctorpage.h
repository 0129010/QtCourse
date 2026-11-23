#ifndef DOCTORPAGE_H
#define DOCTORPAGE_H

#include <QWidget>
#include <QString>

class QPushButton;
class QTableView;
class QSqlTableModel;
class QLabel;

/*
 * 医生信息管理（选做部分）：医生表通过外键 DEPARTMENT_ID 关联科室表，
 * 科室列使用下拉框代理，用户可以直接在表格里把医生调整到其他科室。
 */
class DoctorPage : public QWidget
{
    Q_OBJECT
public:
    explicit DoctorPage(const QString &userId, QWidget *parent = nullptr);

public slots:
    void refresh();
    void onAddClicked();
    void onDeleteClicked();

private:
    void initUi();
    void setupModel();
    void updateStatus(const QString &extra = QString());

    QString m_userId;
    QTableView *m_tableView;
    QSqlTableModel *m_model;
    QLabel *m_statusLabel;
    QPushButton *m_addButton;
    QPushButton *m_deleteButton;
    QPushButton *m_refreshButton;
};

#endif // DOCTORPAGE_H
