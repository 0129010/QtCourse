#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

class QStackedWidget;
class QToolBar;
class QLabel;
class PatientPage;
class DoctorPage;
class DepartmentPage;
class HistoryPage;

/*
 * 主窗口：用 QStackedWidget 组织“患者管理 / 医生管理 / 科室管理 / 操作记录”四个页面，
 * 工具栏的按钮全部由代码创建（不使用 .ui），点击后切换 stacked widget 的当前页。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(const QString &userId, const QString &userFullName,
                        QWidget *parent = nullptr);
    ~MainWindow() override;

    void switchToPage(int index);
    PatientPage *patientPage() const { return m_patientPage; }
    DoctorPage *doctorPage() const { return m_doctorPage; }
    DepartmentPage *departmentPage() const { return m_departmentPage; }
    HistoryPage *historyPage() const { return m_historyPage; }

private:
    void createToolBar();
    void createStatusBar();

    QString m_userId;
    QString m_userFullName;

    QStackedWidget *m_stack;
    QToolBar *m_toolBar;
    QLabel *m_userLabel;
    QLabel *m_dbLabel;

    PatientPage *m_patientPage;
    DoctorPage *m_doctorPage;
    DepartmentPage *m_departmentPage;
    HistoryPage *m_historyPage;
};

#endif // MAINWINDOW_H
