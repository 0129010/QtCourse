#include "mainwindow.h"

#include "db/databasemanager.h"
#include "ui/departmentpage.h"
#include "ui/doctorpage.h"
#include "ui/historypage.h"
#include "ui/patientpage.h"
#include "util/messagebox.h"

#include <QAction>
#include <QActionGroup>
#include <QLabel>
#include <QStatusBar>
#include <QStackedWidget>
#include <QToolBar>
#include <QToolButton>

namespace {
// 学生信息：实验要求界面上必须出现学号与姓名
const QString kStudentId = QStringLiteral("2024414300103");
const QString kStudentName = QStringLiteral("高俊");
} // namespace

MainWindow::MainWindow(const QString &userId, const QString &userFullName, QWidget *parent)
    : QMainWindow(parent)
    , m_userId(userId)
    , m_userFullName(userFullName)
    , m_stack(nullptr)
    , m_toolBar(nullptr)
    , m_userLabel(nullptr)
    , m_dbLabel(nullptr)
    , m_patientPage(nullptr)
    , m_doctorPage(nullptr)
    , m_departmentPage(nullptr)
    , m_historyPage(nullptr)
{
    setWindowTitle(QStringLiteral("医院诊疗信息管理系统"));
    resize(1080, 700);

    m_stack = new QStackedWidget(this);
    m_patientPage = new PatientPage(m_userId, m_stack);
    m_doctorPage = new DoctorPage(m_userId, m_stack);
    m_departmentPage = new DepartmentPage(m_userId, m_stack);
    m_historyPage = new HistoryPage(m_stack);

    m_stack->addWidget(m_patientPage);
    m_stack->addWidget(m_doctorPage);
    m_stack->addWidget(m_departmentPage);
    m_stack->addWidget(m_historyPage);
    setCentralWidget(m_stack);

    createToolBar();
    createStatusBar();
    switchToPage(0);
}

MainWindow::~MainWindow()
{
}

void MainWindow::createToolBar()
{
    m_toolBar = new QToolBar(QStringLiteral("功能导航"), this);
    m_toolBar->setObjectName(QStringLiteral("mainToolBar"));
    m_toolBar->setMovable(false);
    m_toolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    addToolBar(Qt::TopToolBarArea, m_toolBar);

    QActionGroup *group = new QActionGroup(this);
    group->setExclusive(true);

    struct PageAction { const char *text; int index; };
    const PageAction actions[] = {
        { "患者管理", 0 },
        { "医生管理", 1 },
        { "科室管理", 2 },
        { "操作记录", 3 }
    };

    for (const PageAction &item : actions) {
        QAction *action = new QAction(QString::fromUtf8(item.text), this);
        action->setCheckable(true);
        action->setData(item.index);
        group->addAction(action);
        connect(action, &QAction::triggered, this, [this](bool checked) {
            QAction *source = qobject_cast<QAction *>(sender());
            if (source) {
                source->setChecked(checked);
                switchToPage(source->data().toInt());
            }
        });
        m_toolBar->addAction(action);
        if (item.index == 0) {
            action->setChecked(true);
        }
    }

    m_toolBar->addSeparator();
    QAction *quitAction = new QAction(QStringLiteral("退出系统"), this);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);
    m_toolBar->addAction(quitAction);
}

void MainWindow::createStatusBar()
{
    m_userLabel = new QLabel(QStringLiteral("当前用户：%1（%2）").arg(m_userFullName, m_userId), this);
    statusBar()->addWidget(m_userLabel);

    m_dbLabel = new QLabel(QStringLiteral("数据库：%1").arg(DatabaseManager::instance().dbPath()), this);
    statusBar()->addPermanentWidget(m_dbLabel);

    // 学号、姓名按要求固定在界面右上角的状态栏
    QLabel *student = new QLabel(QStringLiteral("学号：%1    姓名：%2").arg(kStudentId, kStudentName), this);
    student->setObjectName(QStringLiteral("studentLabel"));
    student->setStyleSheet(QStringLiteral("color:#8e44ad; font-weight:bold; padding-right:8px;"));
    statusBar()->addPermanentWidget(student);
}

void MainWindow::switchToPage(int index)
{
    if (index < 0 || index >= m_stack->count()) {
        return;
    }
    m_stack->setCurrentIndex(index);

    for (QAction *action : m_toolBar->actions()) {
        if (action->isCheckable() && action->data().toInt() == index) {
            action->setChecked(true);
        }
    }

    if (index == 3 && m_historyPage) {
        m_historyPage->refresh();
    }
}
