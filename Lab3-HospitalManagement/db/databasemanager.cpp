#include "databasemanager.h"

#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <QVariant>

namespace {
const char *kConnectionName = "clinic_connection";

// 首次运行时写入的演示数据（姓名、身份证、手机号均为演示数据）
struct PatientSeed
{
    const char *id;
    const char *idCard;
    QString name;
    int sex;        // 1: 男, 0: 女
    const char *dob;
    double height;
    double weight;
    const char *phone;
    int age;
    const char *created;
};

const PatientSeed kPatients[] = {
    { "P0001", "441900199003120011", QStringLiteral("王建国"), 1, "1990-03-12", 175.0, 72.5, "13800138001", 36, "2026-09-08 09:12:30" },
    { "P0002", "441900199807200022", QStringLiteral("李秀英"), 0, "1998-07-20", 162.0, 55.0, "13900139002", 28, "2026-09-09 10:35:11" },
    { "P0003", "441900200105050033", QStringLiteral("王小明"), 1, "2001-05-05", 178.5, 80.0, "13700137003", 25, "2026-09-12 14:02:45" },
    { "P0004", "441900197511230044", QStringLiteral("张桂芳"), 0, "1975-11-23", 158.0, 61.2, "13600136004", 50, "2026-09-15 08:20:09" },
    { "P0005", "441900201204180055", QStringLiteral("陈思远"), 1, "2012-04-18", 140.0, 38.5, "13500135005", 14, "2026-09-18 16:41:27" },
    { "P0006", "441900199501010066", QStringLiteral("刘洋"),   1, "1995-01-01", 172.0, 68.0, "13400134006", 31, "2026-09-20 11:07:53" },
    { "P0007", "441900198809090077", QStringLiteral("王丽"),   0, "1988-09-09", 165.0, 58.8, "13300133007", 38, "2026-09-25 09:55:16" },
    { "P0008", "441900200306300088", QStringLiteral("赵一鸣"), 1, "2003-06-30", 180.0, 75.6, "13200132008", 23, "2026-10-02 13:28:40" }
};

struct DoctorSeed
{
    const char *id;
    const char *no;
    QString name;
    const char *dept;
};

const DoctorSeed kDoctors[] = {
    { "D0001", "E2023001", QStringLiteral("张建国"), "DP01" },
    { "D0002", "E2023002", QStringLiteral("李慧"),   "DP02" },
    { "D0003", "E2023003", QStringLiteral("王强"),   "DP03" }
};
} // namespace

DatabaseManager &DatabaseManager::instance()
{
    static DatabaseManager manager;
    return manager;
}

DatabaseManager::DatabaseManager()
    : m_open(false)
{
}

DatabaseManager::~DatabaseManager()
{
    if (m_open) {
        {
            QSqlDatabase db = QSqlDatabase::database(kConnectionName, false);
            if (db.isValid()) {
                db.close();
            }
        }
        QSqlDatabase::removeDatabase(kConnectionName);
        m_open = false;
    }
}

bool DatabaseManager::open(const QString &dbPath, QString *errorText)
{
    if (m_open && m_dbPath == dbPath) {
        return true;
    }

    if (QSqlDatabase::contains(kConnectionName)) {
        {
            QSqlDatabase old = QSqlDatabase::database(kConnectionName, false);
            if (old.isValid()) {
                old.close();
            }
        }
        QSqlDatabase::removeDatabase(kConnectionName);
    }

    if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        m_lastError = QStringLiteral("当前 Qt 未安装 QSQLITE 驱动，无法打开数据库。");
        if (errorText) {
            *errorText = m_lastError;
        }
        return false;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", kConnectionName);
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        m_lastError = QStringLiteral("数据库打开失败：%1").arg(db.lastError().text());
        if (errorText) {
            *errorText = m_lastError;
        }
        return false;
    }

    m_dbPath = dbPath;
    m_open = true;

    QSqlQuery pragma(db);
    pragma.exec(QStringLiteral("PRAGMA foreign_keys = ON"));

    QString err;
    if (!createTables(&err) || !seedData()) {
        m_lastError = err.isEmpty() ? QStringLiteral("数据库初始化失败。") : err;
        if (errorText) {
            *errorText = m_lastError;
        }
        return false;
    }
    return true;
}

QSqlDatabase DatabaseManager::database() const
{
    return QSqlDatabase::database(kConnectionName, false);
}

bool DatabaseManager::execSql(const QString &sql, QString *errorText)
{
    QSqlQuery query(database());
    if (!query.exec(sql)) {
        const QString msg = QStringLiteral("执行 SQL 失败：%1\n%2").arg(sql, query.lastError().text());
        m_lastError = msg;
        if (errorText) {
            *errorText = msg;
        }
        return false;
    }
    return true;
}

bool DatabaseManager::createTables(QString *errorText)
{
    const QStringList ddl = {
        QStringLiteral("CREATE TABLE IF NOT EXISTS user ("
                       "ID TEXT PRIMARY KEY, FULLNAME TEXT NOT NULL, "
                       "USERNAME TEXT NOT NULL UNIQUE, PASSWORD TEXT NOT NULL)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS department ("
                       "ID TEXT PRIMARY KEY, NAME TEXT NOT NULL)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS doctor ("
                       "ID TEXT PRIMARY KEY, EMPLOYEE_NO TEXT, NAME TEXT NOT NULL, "
                       "DEPARTMENT_ID TEXT, FOREIGN KEY(DEPARTMENT_ID) REFERENCES department(ID))"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS patient ("
                       "ID TEXT PRIMARY KEY, ID_CARD TEXT, NAME TEXT NOT NULL, SEX INTEGER, "
                       "DOB TEXT, HEIGHT REAL, WEIGHT REAL, MOBILEPHONE TEXT, AGE INTEGER, "
                       "CREATEDTIMESTAMP TEXT)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS history ("
                       "ID TEXT PRIMARY KEY, USER_ID TEXT, EVENT TEXT, TIMESTAMP TEXT, "
                       "FOREIGN KEY(USER_ID) REFERENCES user(ID))")
    };

    for (const QString &sql : ddl) {
        if (!execSql(sql, errorText)) {
            return false;
        }
    }
    return true;
}

bool DatabaseManager::seedData()
{
    QSqlDatabase db = database();
    QSqlQuery check(db);

    // 用户表：登录账号
    check.exec(QStringLiteral("SELECT COUNT(*) FROM user"));
    if (check.next() && check.value(0).toInt() == 0) {
        QSqlQuery insert(db);
        insert.prepare(QStringLiteral("INSERT INTO user (ID, FULLNAME, USERNAME, PASSWORD) VALUES (?, ?, ?, ?)"));
        insert.addBindValue(QStringLiteral("U0001"));
        insert.addBindValue(QStringLiteral("系统管理员"));
        insert.addBindValue(QStringLiteral("admin"));
        insert.addBindValue(QStringLiteral("123456"));
        insert.exec();

        insert.addBindValue(QStringLiteral("U0002"));
        insert.addBindValue(QStringLiteral("门诊医生"));
        insert.addBindValue(QStringLiteral("doctor"));
        insert.addBindValue(QStringLiteral("123456"));
        insert.exec();
    }
    check.finish();

    // 科室与医生
    check.exec(QStringLiteral("SELECT COUNT(*) FROM department"));
    if (check.next() && check.value(0).toInt() == 0) {
        QSqlQuery insert(db);
        insert.prepare(QStringLiteral("INSERT INTO department (ID, NAME) VALUES (?, ?)"));
        const QStringList names = { QStringLiteral("内科"), QStringLiteral("外科"), QStringLiteral("儿科") };
        for (int i = 0; i < names.size(); ++i) {
            insert.addBindValue(QStringLiteral("DP%1").arg(i + 1, 2, 10, QLatin1Char('0')));
            insert.addBindValue(names.at(i));
            insert.exec();
        }

        insert.prepare(QStringLiteral("INSERT INTO doctor (ID, EMPLOYEE_NO, NAME, DEPARTMENT_ID) VALUES (?, ?, ?, ?)"));
        for (const DoctorSeed &d : kDoctors) {
            insert.addBindValue(QString::fromUtf8(d.id));
            insert.addBindValue(QString::fromUtf8(d.no));
            insert.addBindValue(d.name);
            insert.addBindValue(QString::fromUtf8(d.dept));
            insert.exec();
        }
    }
    check.finish();

    // 患者
    check.exec(QStringLiteral("SELECT COUNT(*) FROM patient"));
    if (check.next() && check.value(0).toInt() == 0) {
        QSqlQuery insert(db);
        insert.prepare(QStringLiteral(
            "INSERT INTO patient (ID, ID_CARD, NAME, SEX, DOB, HEIGHT, WEIGHT, MOBILEPHONE, AGE, CREATEDTIMESTAMP) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));
        for (const PatientSeed &p : kPatients) {
            insert.addBindValue(QString::fromUtf8(p.id));
            insert.addBindValue(QString::fromUtf8(p.idCard));
            insert.addBindValue(p.name);
            insert.addBindValue(p.sex);
            insert.addBindValue(QString::fromUtf8(p.dob));
            insert.addBindValue(p.height);
            insert.addBindValue(p.weight);
            insert.addBindValue(QString::fromUtf8(p.phone));
            insert.addBindValue(p.age);
            insert.addBindValue(QString::fromUtf8(p.created));
            if (!insert.exec()) {
                m_lastError = QStringLiteral("写入初始患者数据失败：%1").arg(insert.lastError().text());
                return false;
            }
        }
    }
    return true;
}

bool DatabaseManager::login(const QString &username, const QString &password,
                            QString *fullName, QString *userId)
{
    QSqlQuery query(database());
    query.prepare(QStringLiteral("SELECT ID, FULLNAME FROM user WHERE USERNAME = ? AND PASSWORD = ?"));
    query.addBindValue(username);
    query.addBindValue(password);
    if (!query.exec()) {
        m_lastError = QStringLiteral("登录查询失败：%1").arg(query.lastError().text());
        return false;
    }
    if (!query.next()) {
        return false;
    }
    if (userId) {
        *userId = query.value(0).toString();
    }
    if (fullName) {
        *fullName = query.value(1).toString();
    }
    return true;
}

void DatabaseManager::logEvent(const QString &userId, const QString &event)
{
    QSqlQuery query(database());
    query.prepare(QStringLiteral("INSERT INTO history (ID, USER_ID, EVENT, TIMESTAMP) VALUES (?, ?, ?, ?)"));
    QString id = QUuid::createUuid().toString().remove(QLatin1Char('{')).remove(QLatin1Char('}'));
    id.remove(QLatin1Char('-'));
    query.addBindValue(QStringLiteral("H") + id.left(12));
    query.addBindValue(userId);
    query.addBindValue(event);
    query.addBindValue(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));
    query.exec();
}

QString DatabaseManager::nextId(const QString &table, const QString &prefix, int width)
{
    QSqlQuery query(database());
    query.prepare(QStringLiteral("SELECT ID FROM %1 WHERE ID LIKE ? ORDER BY ID DESC LIMIT 1").arg(table));
    query.addBindValue(prefix + QLatin1Char('%'));
    int number = 0;
    if (query.exec() && query.next()) {
        number = query.value(0).toString().mid(prefix.size()).toInt();
    }
    return prefix + QStringLiteral("%1").arg(number + 1, width, 10, QLatin1Char('0'));
}
