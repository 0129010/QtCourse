#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>

/*
 * 数据库访问层的单例（本实验的 Model 层之一）：
 *  - 负责 QSQLITE 驱动的连接、建表与初始化数据
 *  - 向上层提供登录校验、操作记录写入等通用服务
 * 所有界面代码都不直接 new QSqlDatabase，统一从 instance().database() 取连接。
 */
class DatabaseManager : QObject
{
public:
    static DatabaseManager &instance();

    bool open(const QString &dbPath, QString *errorText = nullptr);
    QSqlDatabase database() const;
    bool isOpen() const { return m_open; }

    // 登录校验：成功时通过 fullName 返回用户的显示名，user 表中的 ID 由 *userId 返回
    bool login(const QString &username, const QString &password,
               QString *fullName = nullptr, QString *userId = nullptr);

    // 记录一次用户操作（写入 history 表）
    void logEvent(const QString &userId, const QString &event);

    // 生成形如 P0009、D0004 的主键
    QString nextId(const QString &table, const QString &prefix, int width = 4);

    QString dbPath() const { return m_dbPath; }
    QString lastError() const { return m_lastError; }

private:
    DatabaseManager();
    ~DatabaseManager();

    bool createTables(QString *errorText);
    bool seedData();
    bool execSql(const QString &sql, QString *errorText = nullptr);

    QString m_dbPath;
    QString m_lastError;
    bool m_open;
};

#endif // DATABASEMANAGER_H
