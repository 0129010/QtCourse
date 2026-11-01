#include "finddialog.h"
#include "ui_finddialog.h"

#include "codeeditor.h"

#include <QApplication>
#include <QTextBlock>

namespace {
// 高亮与统计都需要遍历全文，超大文档必须设上限，否则界面会卡住
const int kMaxHighlight = 500;
const int kMaxCountScan = 20000;
}

FindDialog::FindDialog(CodeEditor *editor, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FindDialog)
    , m_editor(editor)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setModal(false);

    connect(ui->pushButtonFindNext, &QPushButton::clicked, this, &FindDialog::findNext);
    connect(ui->pushButtonClose, &QPushButton::clicked, this, &FindDialog::close);
    connect(ui->lineEditFind, &QLineEdit::returnPressed, this, &FindDialog::findNext);
    connect(ui->lineEditFind, &QLineEdit::textChanged,
            this, &FindDialog::handleFindTextChanged);
    connect(ui->checkBoxCase, &QCheckBox::toggled, this, &FindDialog::refreshHighlights);
    connect(ui->checkBoxWholeWord, &QCheckBox::toggled, this, &FindDialog::refreshHighlights);
    connect(ui->checkBoxHighlight, &QCheckBox::toggled, this, &FindDialog::refreshHighlights);

    ui->checkBoxHighlight->setChecked(true);
}

FindDialog::~FindDialog()
{
    delete ui;
}

void FindDialog::setSearchText(const QString &text)
{
    if (text.isEmpty() || text.contains(QChar::ParagraphSeparator)
            || text.contains(QLatin1Char('\n')))
        return;

    ui->lineEditFind->setText(text);
    ui->lineEditFind->selectAll();
    ui->labelMessage->setText(tr("已带入编辑器选中的文字，按回车开始查找。"));
    refreshHighlights();
}

QString FindDialog::searchText() const
{
    return ui->lineEditFind->text();
}

QTextDocument::FindFlags FindDialog::currentFlags(bool backward) const
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

void FindDialog::findNext()
{
    performSearch(ui->checkBoxBackward->isChecked());
}

void FindDialog::findPrevious()
{
    performSearch(true);
}

bool FindDialog::performSearch(bool backward)
{
    const QString text = ui->lineEditFind->text();
    if (text.isEmpty()) {
        ui->labelMessage->setText(tr("查找内容不能为空，请先输入要查找的文字。"));
        ui->lineEditFind->setFocus();
        return false;
    }

    QTextDocument *doc = m_editor->document();
    const QTextDocument::FindFlags flags = currentFlags(backward);

    // 连续查找的关键：以“上次匹配的选区边界”作为本次起点，而不是光标原点
    const QTextCursor current = m_editor->textCursor();
    const int from = backward ? current.selectionStart() : current.selectionEnd();

    QTextCursor found = doc->find(text, from, flags);
    bool wrapped = false;

    if (found.isNull()) {
        // 到达文档首/尾：回绕到另一端再找一次
        const int wrapFrom = backward ? doc->characterCount() - 1 : 0;
        found = doc->find(text, wrapFrom, flags);
        wrapped = !found.isNull();
    }

    if (found.isNull()) {
        ui->labelMessage->setText(tr("未找到“%1”。").arg(text));
        QApplication::beep();
        applyHighlights();
        emit searchFinished(false);
        return false;
    }

    m_editor->setTextCursor(found);
    m_editor->ensureCursorVisible();
    updateMatchInfo(found, wrapped);
    applyHighlights();
    emit searchFinished(true);
    return true;
}

void FindDialog::updateMatchInfo(const QTextCursor &found, bool wrapped)
{
    QTextDocument *doc = m_editor->document();
    const QString text = ui->lineEditFind->text();
    const QTextDocument::FindFlags flags = currentFlags(false);

    int total = 0;
    int before = 0;
    int from = 0;
    while (total < kMaxCountScan) {
        const QTextCursor hit = doc->find(text, from, flags);
        if (hit.isNull())
            break;
        ++total;
        if (hit.selectionStart() < found.selectionStart())
            ++before;
        from = hit.position();
        if (hit.selectionStart() == hit.selectionEnd())
            ++from; // 防御：零宽匹配时强制前进，避免死循环
    }

    QString message;
    if (total >= kMaxCountScan)
        message = tr("第 %1 处 / 匹配过多（≥%2 处）").arg(before + 1).arg(kMaxCountScan);
    else
        message = tr("第 %1 处 / 共 %2 处匹配").arg(before + 1).arg(total);

    if (wrapped)
        message += tr("；已回绕到文档另一端继续查找");

    ui->labelMessage->setText(message);
}

void FindDialog::applyHighlights()
{
    m_editor->clearSearchHighlights();

    const QString text = ui->lineEditFind->text();
    if (text.isEmpty() || !ui->checkBoxHighlight->isChecked())
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

void FindDialog::refreshHighlights()
{
    applyHighlights();
}

void FindDialog::handleFindTextChanged()
{
    if (ui->lineEditFind->text().isEmpty()) {
        m_editor->clearSearchHighlights();
        ui->labelMessage->setText(tr("提示：重复点击“查找下一个”可连续向后查找"));
        return;
    }
    applyHighlights();
}

void FindDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    ui->lineEditFind->setFocus();
    ui->lineEditFind->selectAll();
}

void FindDialog::closeEvent(QCloseEvent *event)
{
    // 关闭查找窗口时把高亮清干净，否则会一直留在文档上
    m_editor->clearSearchHighlights();
    QDialog::closeEvent(event);
}
