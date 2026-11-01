#include "replacedialog.h"
#include "ui_replacedialog.h"

#include "codeeditor.h"

#include <QApplication>

namespace {
const int kMaxHighlight = 500;
// 防御上限：即使替换逻辑异常也不会把程序卡死
const int kMaxReplaceCount = 100000;
}

ReplaceDialog::ReplaceDialog(CodeEditor *editor, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ReplaceDialog)
    , m_editor(editor)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setModal(false);

    connect(ui->pushButtonFindNext, &QPushButton::clicked, this, &ReplaceDialog::findNext);
    connect(ui->pushButtonReplace, &QPushButton::clicked, this, &ReplaceDialog::replaceOne);
    connect(ui->pushButtonReplaceAll, &QPushButton::clicked, this, &ReplaceDialog::replaceAll);
    connect(ui->pushButtonClose, &QPushButton::clicked, this, &ReplaceDialog::close);
    connect(ui->lineEditFind, &QLineEdit::textChanged,
            this, &ReplaceDialog::handleFindTextChanged);
    connect(ui->checkBoxCase, &QCheckBox::toggled, this, &ReplaceDialog::applyHighlights);
    connect(ui->checkBoxWholeWord, &QCheckBox::toggled, this, &ReplaceDialog::applyHighlights);
}

ReplaceDialog::~ReplaceDialog()
{
    delete ui;
}

void ReplaceDialog::setSearchText(const QString &text)
{
    if (text.isEmpty() || text.contains(QChar::ParagraphSeparator))
        return;
    ui->lineEditFind->setText(text);
    ui->lineEditFind->selectAll();
    applyHighlights();
}

QTextDocument::FindFlags ReplaceDialog::currentFlags(bool backward) const
{
    QTextDocument::FindFlags flags;
    if (ui->checkBoxCase->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if (ui->checkBoxWholeWord->isChecked())
        flags |= QTextDocument::FindWholeWords;
    if (backward)
        flags |= QTextDocument::FindBackward;
    return flags;
}

void ReplaceDialog::findNext()
{
    performSearch(false);
}

bool ReplaceDialog::performSearch(bool backward)
{
    const QString text = ui->lineEditFind->text();
    if (text.isEmpty()) {
        ui->labelMessage->setText(tr("查找内容不能为空，请先输入要查找的文字。"));
        ui->lineEditFind->setFocus();
        return false;
    }

    QTextDocument *doc = m_editor->document();
    const QTextDocument::FindFlags flags = currentFlags(backward);
    const QTextCursor current = m_editor->textCursor();
    const int from = backward ? current.selectionStart() : current.selectionEnd();

    QTextCursor found = doc->find(text, from, flags);
    bool wrapped = false;
    if (found.isNull()) {
        const int wrapFrom = backward ? doc->characterCount() - 1 : 0;
        found = doc->find(text, wrapFrom, flags);
        wrapped = !found.isNull();
    }

    if (found.isNull()) {
        ui->labelMessage->setText(tr("未找到“%1”，没有可替换的内容。").arg(text));
        QApplication::beep();
        return false;
    }

    m_editor->setTextCursor(found);
    m_editor->ensureCursorVisible();
    applyHighlights();
    ui->labelMessage->setText(wrapped
        ? tr("已回绕到文档另一端，定位到一处匹配。")
        : tr("已定位到一处匹配，点“替换”只改这一处。"));
    return true;
}

void ReplaceDialog::replaceOne()
{
    const QString find = ui->lineEditFind->text();
    if (find.isEmpty()) {
        ui->labelMessage->setText(tr("查找内容不能为空，请先输入要查找的文字。"));
        ui->lineEditFind->setFocus();
        return;
    }

    const QString replacement = ui->lineEditReplace->text();
    QTextCursor cursor = m_editor->textCursor();

    // 当前选区真的是要找的内容才替换，否则先定位到下一处
    const QString selected = cursor.selectedText();
    const bool caseSensitive = ui->checkBoxCase->isChecked();
    const bool sameAsQuery = cursor.hasSelection()
            && (caseSensitive ? selected == find
                              : selected.compare(find, Qt::CaseInsensitive) == 0);

    if (sameAsQuery) {
        cursor.insertText(replacement);
        m_editor->setTextCursor(cursor);
        applyHighlights();
        ui->labelMessage->setText(tr("已替换 1 处。"));
        performSearch(false);
        return;
    }

    performSearch(false);
}

void ReplaceDialog::replaceAll()
{
    const QString find = ui->lineEditFind->text();
    if (find.isEmpty()) {
        ui->labelMessage->setText(tr("查找内容不能为空，请先输入要查找的文字。"));
        ui->lineEditFind->setFocus();
        return;
    }

    const QString replacement = ui->lineEditReplace->text();
    QTextDocument *doc = m_editor->document();
    const QTextDocument::FindFlags flags = currentFlags(false);

    int replaced = 0;
    QTextCursor editCursor(doc);
    editCursor.beginEditBlock();

    int from = 0;
    QTextCursor hit = doc->find(find, from, flags);
    while (!hit.isNull() && replaced < kMaxReplaceCount) {
        hit.insertText(replacement);
        ++replaced;

        // 关键：从替换后光标的新位置继续，替换串包含查找串时也不会原地打转；
        // 替换成空串时光标停在原处，下一次查找自然跳过被删掉的内容
        from = hit.position();
        hit = doc->find(find, from, flags);
    }

    editCursor.endEditBlock();
    applyHighlights();

    if (replaced == 0) {
        ui->labelMessage->setText(tr("未找到“%1”，没有做任何替换。").arg(find));
        QApplication::beep();
        return;
    }

    if (replaced >= kMaxReplaceCount)
        ui->labelMessage->setText(tr("已达到单次替换上限 %1 处，请检查替换内容。").arg(kMaxReplaceCount));
    else
        ui->labelMessage->setText(tr("全部替换完成，共替换 %1 处。").arg(replaced));
}

void ReplaceDialog::applyHighlights()
{
    m_editor->clearSearchHighlights();

    const QString text = ui->lineEditFind->text();
    if (text.isEmpty())
        return;

    QTextDocument *doc = m_editor->document();
    const QTextDocument::FindFlags flags = currentFlags(false);
    const QColor highlight(255, 226, 128);

    QList<QTextEdit::ExtraSelection> selections;
    int from = 0;
    int count = 0;
    while (count < kMaxHighlight) {
        const QTextCursor hit = doc->find(text, from, flags);
        if (hit.isNull())
            break;

        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(highlight);
        selection.cursor = hit;
        selections.append(selection);

        from = hit.position();
        if (hit.selectionStart() == hit.selectionEnd())
            ++from;
        ++count;
    }

    m_editor->setSearchHighlights(selections);
}

void ReplaceDialog::handleFindTextChanged()
{
    if (ui->lineEditFind->text().isEmpty()) {
        m_editor->clearSearchHighlights();
        ui->labelMessage->setText(tr("提示：“替换”只替换当前匹配处，“全部替换”会统计并替换全文"));
        return;
    }
    applyHighlights();
}

void ReplaceDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    ui->lineEditFind->setFocus();
    ui->lineEditFind->selectAll();
}

void ReplaceDialog::closeEvent(QCloseEvent *event)
{
    m_editor->clearSearchHighlights();
    QDialog::closeEvent(event);
}
