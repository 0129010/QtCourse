#include "doublespindelegate.h"

#include <QDoubleSpinBox>

DoubleSpinDelegate::DoubleSpinDelegate(double min, double max, double step,
                                      int decimals, QObject *parent)
    : QStyledItemDelegate(parent)
    , m_min(min)
    , m_max(max)
    , m_step(step)
    , m_decimals(decimals)
{
}

QWidget *DoubleSpinDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
    editor->setRange(m_min, m_max);
    editor->setSingleStep(m_step);
    editor->setDecimals(m_decimals);
    return editor;
}

void DoubleSpinDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QDoubleSpinBox *spin = qobject_cast<QDoubleSpinBox *>(editor);
    if (!spin) {
        return;
    }
    spin->setValue(index.data(Qt::EditRole).toDouble());
}

void DoubleSpinDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                      const QModelIndex &index) const
{
    QDoubleSpinBox *spin = qobject_cast<QDoubleSpinBox *>(editor);
    if (!spin) {
        return;
    }
    spin->interpretText();
    model->setData(index, spin->value(), Qt::EditRole);
}

void DoubleSpinDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                              const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}
