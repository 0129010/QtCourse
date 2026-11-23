#ifndef PATIENTTABLEMODEL_H
#define PATIENTTABLEMODEL_H

#include <QSqlTableModel>

/*
 * 患者表的专用 Model：在 QSqlTableModel 的基础上做一层“数据语义转换”。
 * 数据库里性别用整数保存（1 男 / 0 女），但界面显示应该是“男/女”；
 * 因此重写 data() 负责显示映射、重写 setData() 负责把下拉框选中的
 * “男/女”反转换回整数，保证写回数据库的类型与表结构一致。
 */
class PatientTableModel : public QSqlTableModel
{
    Q_OBJECT
public:
    explicit PatientTableModel(QObject *parent, QSqlDatabase db);

    enum Column { ColumnId = 0, ColumnIdCard = 1, ColumnName = 2, ColumnSex = 3 };

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

private:
    static QString sexToText(int sex);
    static int textToSex(const QVariant &value);
};

#endif // PATIENTTABLEMODEL_H
