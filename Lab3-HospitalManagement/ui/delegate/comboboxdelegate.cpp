#include "comboboxdelegate.h"

#include <QComboBox>

ComboBoxDelegate::ComboBoxDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void ComboBoxDelegate::setItems(const QStringList &labels, const QStringList &values)
{
    m_labels = labels;
    m_values = values;
}

QWidget *ComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    QComboBox *editor = new QComboBox(parent);
    for (int i = 0; i < m_labels.size(); ++i) {
        editor->addItem(m_labels.at(i), m_values.value(i));
    }
    return editor;
}

void ComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *combo = qobject_cast<QComboBox *>(editor);
    if (!combo) {
        return;
    }
    const QString value = index.data(Qt::EditRole).toString();
    const int pos = combo->findData(value);
    combo->setCurrentIndex(pos < 0 ? 0 : pos);
}

void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    QComboBox *combo = qobject_cast<QComboBox *>(editor);
    if (!combo) {
        return;
    }
    model->setData(index, combo->currentData().toString(), Qt::EditRole);
}

void ComboBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}
