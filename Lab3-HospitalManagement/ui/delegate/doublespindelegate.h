#ifndef DOUBLESPINDELEGATE_H
#define DOUBLESPINDELEGATE_H

#include <QStyledItemDelegate>

/*
 * 双精度数值代理：身高、体重列在 TableView 中用 QDoubleSpinBox 编辑，
 * 并限制取值范围，避免在视图里直接输入非法数值。
 */
class DoubleSpinDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DoubleSpinDelegate(double min, double max, double step,
                                int decimals, QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

private:
    double m_min;
    double m_max;
    double m_step;
    int m_decimals;
};

#endif // DOUBLESPINDELEGATE_H
