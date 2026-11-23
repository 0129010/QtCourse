#ifndef COMBOBOXDELEGATE_H
#define COMBOBOXDELEGATE_H

#include <QStyledItemDelegate>
#include <QStringList>

/*
 * 下拉框代理：把某一列（性别、科室）在 TableView 中以内嵌下拉框的形式编辑。
 * labels 与 values 一一对应，例如 性别：{"男", "女"} <-> {"1", "0"}。
 */
class ComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ComboBoxDelegate(QObject *parent = nullptr);

    void setItems(const QStringList &labels, const QStringList &values);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

private:
    QStringList m_labels;
    QStringList m_values;
};

#endif // COMBOBOXDELEGATE_H
