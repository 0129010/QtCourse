#ifndef HISTORYPAGE_H
#define HISTORYPAGE_H

#include <QWidget>

class QTableView;
class QSqlQueryModel;
class QLabel;

/*
 * 操作记录（选做部分）：用 QSqlQueryModel 执行一条带连接的查询，
 * 把 history 表与 user 表关联，按时间倒序显示“谁在什么时候做了什么”。
 */
class HistoryPage : public QWidget
{
    Q_OBJECT
public:
    explicit HistoryPage(QWidget *parent = nullptr);

public slots:
    void refresh();

private:
    void initUi();

    QTableView *m_tableView;
    QSqlQueryModel *m_model;
    QLabel *m_statusLabel;
};

#endif // HISTORYPAGE_H
