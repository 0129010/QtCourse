#include "db/databasemanager.h"
#include "ui/logindialog.h"
#include "ui/mainwindow.h"
#include "ui/patientdialog.h"
#include "ui/patientpage.h"
#include "util/messagebox.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QMessageBox>
#include <QPixmap>
#include <QStringList>
#include <QVariantMap>

namespace {

const char *kDbFileName = "clinic.db";

QString dbErrorText(const QString &detail)
{
    return QStringLiteral("无法打开数据库文件“%1”。\n%2\n\n请检查程序目录是否有写入权限，"
                          "以及 Qt 的 QSQLITE 驱动是否已正确部署。")
        .arg(kDbFileName, detail);
}

// 把控件渲染成 PNG：先 show 再 grab，保证布局与样式已生效
bool saveShot(QWidget *widget, const QString &dir, const QString &fileName)
{
    if (!widget) {
        return false;
    }
    widget->show();
    QApplication::processEvents();
    const QPixmap pixmap = widget->grab();
    if (pixmap.isNull()) {
        return false;
    }
    return pixmap.save(QDir(dir).absoluteFilePath(fileName), "PNG");
}

/*
 * 自动截图模式：Lab3.exe --shots <目录>
 * 该机器没有可用的 GUI 自动化工具，因此用 QWidget::grab() 依次渲染
 * 登录页、异常提示、主界面各页面与对话框，供实验报告引用。
 */
int runShotMode(const QString &dir)
{
    QDir().mkpath(dir);
    int saved = 0;
    const QString target = QDir(dir).absolutePath();

    // 1. 登录界面（已填入演示账号）
    {
        LoginDialog dlg;
        dlg.setCredentials(QStringLiteral("admin"), QStringLiteral("123456"));
        if (saveShot(&dlg, target, QStringLiteral("01-login.png"))) {
            ++saved;
        }
    }

    // 2. 登录失败提示（错误密码）
    {
        LoginDialog dlg;
        dlg.setCredentials(QStringLiteral("admin"), QStringLiteral("000000"));
        dlg.submit();
        if (saveShot(&dlg, target, QStringLiteral("02-login-failed.png"))) {
            ++saved;
        }
    }

    // 3. 数据库连接失败提示
    {
        QMessageBox *box = MsgBox::createError(nullptr, QStringLiteral("数据库连接失败"),
                                               dbErrorText(QStringLiteral("unable to open database file")));
        if (saveShot(box, target, QStringLiteral("03-db-error.png"))) {
            ++saved;
        }
        delete box;
    }

    // 4. 主界面：患者列表
    MainWindow window(QStringLiteral("U0001"), QStringLiteral("系统管理员"));
    window.resize(1120, 700);
    window.show();
    QApplication::processEvents();
    window.switchToPage(0);
    if (saveShot(&window, target, QStringLiteral("04-patient-list.png"))) {
        ++saved;
    }

    // 5. 查询命中
    window.patientPage()->setSearchKeyword(QStringLiteral("王"));
    window.patientPage()->onSearchClicked();
    QApplication::processEvents();
    if (saveShot(&window, target, QStringLiteral("05-search-hit.png"))) {
        ++saved;
    }

    // 6. 查询无结果
    window.patientPage()->setSearchKeyword(QStringLiteral("赵六"));
    window.patientPage()->onSearchClicked();
    QApplication::processEvents();
    if (saveShot(&window, target, QStringLiteral("06-search-empty.png"))) {
        ++saved;
    }

    // 恢复列表
    window.patientPage()->setSearchKeyword(QString());
    window.patientPage()->refresh();

    // 7. 添加患者对话框
    {
        QVariantMap initial;
        initial.insert(QStringLiteral("ID"), QStringLiteral("P0009"));
        initial.insert(QStringLiteral("NAME"), QStringLiteral("孙悦"));
        initial.insert(QStringLiteral("ID_CARD"), QStringLiteral("441900200203158765"));
        initial.insert(QStringLiteral("SEX"), 0);
        initial.insert(QStringLiteral("DOB"), QStringLiteral("2002-03-15"));
        initial.insert(QStringLiteral("HEIGHT"), 162.0);
        initial.insert(QStringLiteral("WEIGHT"), 50.5);
        initial.insert(QStringLiteral("MOBILEPHONE"), QStringLiteral("13712345678"));
        PatientDialog dlg(initial, false);
        if (saveShot(&dlg, target, QStringLiteral("07-add-dialog.png"))) {
            ++saved;
        }
    }

    // 8. 必填校验提示
    {
        QMessageBox *box = MsgBox::createWarning(nullptr,
                                                 QStringLiteral("姓名不能为空，请填写患者真实姓名。"));
        if (saveShot(box, target, QStringLiteral("08-validation.png"))) {
            ++saved;
        }
        delete box;
    }

    // 9. 删除确认
    {
        QMessageBox *box = MsgBox::createDeleteConfirm(nullptr, QStringLiteral("患者 王丽（P0007）"));
        if (saveShot(box, target, QStringLiteral("09-delete-confirm.png"))) {
            ++saved;
        }
        delete box;
    }

    // 10-12. 选做页面：医生管理 / 科室管理 / 操作记录
    window.switchToPage(1);
    if (saveShot(&window, target, QStringLiteral("10-doctor-page.png"))) {
        ++saved;
    }
    window.switchToPage(2);
    if (saveShot(&window, target, QStringLiteral("11-department-page.png"))) {
        ++saved;
    }
    window.switchToPage(3);
    if (saveShot(&window, target, QStringLiteral("12-history-page.png"))) {
        ++saved;
    }

    window.close();
    return saved;
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Lab3-Clinic"));
    app.setApplicationDisplayName(QStringLiteral("医院诊疗系统 - 用户信息管理"));

    // 解析 --shots 参数（自动截图模式）
    QString shotDir;
    for (int i = 1; i < argc; ++i) {
        const QString arg = QString::fromLocal8Bit(argv[i]);
        if (arg == QStringLiteral("--shots") && i + 1 < argc) {
            shotDir = QDir::fromNativeSeparators(QString::fromLocal8Bit(argv[i + 1]));
        }
    }

    const QString dbPath = QDir(QCoreApplication::applicationDirPath())
                               .absoluteFilePath(QString::fromLatin1(kDbFileName));
    QString error;
    if (!DatabaseManager::instance().open(dbPath, &error)) {
        if (!shotDir.isEmpty()) {
            QMessageBox *box = MsgBox::createError(nullptr, QStringLiteral("数据库连接失败"),
                                                   dbErrorText(error));
            saveShot(box, shotDir, QStringLiteral("03-db-error.png"));
            delete box;
        } else {
            MsgBox::showError(nullptr, QStringLiteral("数据库连接失败"), dbErrorText(error));
        }
        return 1;
    }

    if (!shotDir.isEmpty()) {
        const int count = runShotMode(shotDir);
        return count > 0 ? 0 : 2;
    }

    // 正常启动流程：先登录，再进入主窗口
    LoginDialog login;
    if (login.exec() != QDialog::Accepted) {
        return 0;
    }

    MainWindow window(login.userId(), login.userFullName());
    window.show();
    return app.exec();
}
