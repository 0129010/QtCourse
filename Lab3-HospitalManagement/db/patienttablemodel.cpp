#include "patienttablemodel.h"

PatientTableModel::PatientTableModel(QObject *parent, QSqlDatabase db)
    : QSqlTableModel(parent, db)
{
}

QString PatientTableModel::sexToText(int sex)
{
    return sex == 1 ? QStringLiteral("男") : QStringLiteral("女");
}

int PatientTableModel::textToSex(const QVariant &value)
{
    const QString text = value.toString().trimmed();
    if (text == QStringLiteral("男")) {
        return 1;
    }
    if (text == QStringLiteral("女")) {
        return 0;
    }
    return text.toInt();
}

QVariant PatientTableModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && index.column() == ColumnSex && role == Qt::DisplayRole) {
        return sexToText(QSqlTableModel::data(index, Qt::EditRole).toInt());
    }
    return QSqlTableModel::data(index, role);
}

bool PatientTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && index.column() == ColumnSex && role == Qt::EditRole) {
        return QSqlTableModel::setData(index, textToSex(value), Qt::EditRole);
    }
    return QSqlTableModel::setData(index, value, role);
}
