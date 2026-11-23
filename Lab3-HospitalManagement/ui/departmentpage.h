#ifndef DEPARTMENTPAGE_H
#define DEPARTMENTPAGE_H

#include <QWidget>
#include <QString>

class QPushButton;
class QTableView;
class QSqlTableModel;
class QLabel;

/*
 * 科室信息管理（选做部分）：新增/删除科室。
 * 删除前会检查是否有医生仍属于该科室，避免破坏外键完整性。
 */
class DepartmentPage : public QWidget
{
    Q_OBJECT
public:
    explicit DepartmentPage(const QString &userId, QWidget *parent = nullptr);

public slots:
    void refresh();
    void onAddClicked();
    void onDeleteClicked();

private:
    void initUi();
    void setupModel();

    QString m_userId;
    QTableView *m_tableView;
    QSqlTableModel *m_model;
    QLabel *m_statusLabel;
};

#endif // DEPARTMENTPAGE_H
