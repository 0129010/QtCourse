#include "codeeditor.h"

#include <QFontMetricsF>
#include <QPainter>
#include <QTextBlock>

// ---------------------------------------------------------------- 行号栏控件
class LineNumberArea : public QWidget
{
public:
    explicit LineNumberArea(CodeEditor *editor)
        : QWidget(editor)
        , m_codeEditor(editor)
    {
        setCursor(Qt::ArrowCursor);
    }

    QSize sizeHint() const override
    {
        return QSize(m_codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        m_codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *m_codeEditor;
};

// ---------------------------------------------------------------- CodeEditor
CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_lineNumberArea(new LineNumberArea(this))
    , m_lineNumberVisible(true)
    , m_currentLineColor(232, 242, 254)
{
    // 等宽字体，中文由 Qt 的字体回退自动补齐
    QFont editorFont(QStringLiteral("Consolas"), 12);
    editorFont.setStyleHint(QFont::Monospace);
    editorFont.setFixedPitch(true);
    setFont(editorFont);
    document()->setDefaultFont(editorFont);

    setTabStopDistance(QFontMetricsF(editorFont).horizontalAdvance(QLatin1Char(' ')) * 4);
    setLineWrapMode(QPlainTextEdit::WidgetWidth);
    setFrameShape(QFrame::NoFrame);

    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest,
            this, &CodeEditor::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged,
            this, &CodeEditor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

int CodeEditor::lineNumberAreaWidth() const
{
    int digits = 1;
    int maxBlock = qMax(1, blockCount());
    while (maxBlock >= 10) {
        maxBlock /= 10;
        ++digits;
    }
    // 左右各留一点空隙，保证 4 位以上行号也不会贴边
    const int space = 14 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void CodeEditor::setLineNumberVisible(bool visible)
{
    if (m_lineNumberVisible == visible)
        return;
    m_lineNumberVisible = visible;
    m_lineNumberArea->setVisible(visible);
    updateLineNumberAreaWidth(0);
    viewport()->update();
}

void CodeEditor::refreshLineNumberWidth()
{
    updateLineNumberAreaWidth(0);
    m_lineNumberArea->update();
}

QColor CodeEditor::editorBackground() const
{
    return palette().color(QPalette::Base);
}

void CodeEditor::setEditorBackground(const QColor &color)
{
    if (!color.isValid())
        return;

    QPalette pal = palette();
    pal.setColor(QPalette::Base, color);
    // 深色背景下自动切换为浅色前景，避免出现“黑底黑字”
    pal.setColor(QPalette::Text, color.lightness() < 128 ? QColor(230, 230, 230)
                                                         : QColor(31, 35, 40));
    setPalette(pal);

    m_currentLineColor = color.lightness() < 128 ? color.lighter(140) : QColor(232, 242, 254);
    highlightCurrentLine();
    viewport()->update();
    m_lineNumberArea->update();
}

void CodeEditor::setSearchHighlights(const QList<QTextEdit::ExtraSelection> &selections)
{
    m_searchSelections = selections;
    updateExtraSelections();
}

void CodeEditor::clearSearchHighlights()
{
    if (m_searchSelections.isEmpty())
        return;
    m_searchSelections.clear();
    updateExtraSelections();
}

void CodeEditor::updateExtraSelections()
{
    // 当前行高亮在第一位，查找高亮追加在后面，两者互不覆盖
    QList<QTextEdit::ExtraSelection> all = m_cursorLineSelection;
    all.append(m_searchSelections);
    setExtraSelections(all);
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    updateLineNumberAreaWidth(0);
}

void CodeEditor::updateLineNumberAreaWidth(int /*newBlockCount*/)
{
    const int width = m_lineNumberVisible ? lineNumberAreaWidth() : 0;
    setViewportMargins(width, 0, 0, 0);

    const QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), width, cr.height()));
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (!m_lineNumberVisible)
        return;

    if (dy != 0) {
        m_lineNumberArea->scroll(0, dy);
    } else {
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::highlightCurrentLine()
{
    m_cursorLineSelection.clear();

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(m_currentLineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        m_cursorLineSelection.append(selection);
    }

    updateExtraSelections();
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_lineNumberArea);

    const QColor base = palette().color(QPalette::Base);
    const bool dark = base.lightness() < 128;
    painter.fillRect(event->rect(), dark ? base.lighter(112) : base.darker(104));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    const QTextBlock currentBlock = textCursor().block();

    const QColor normalColor = dark ? QColor(139, 148, 158) : QColor(154, 162, 173);
    const QColor activeColor = dark ? QColor(127, 179, 255) : QColor(26, 115, 232);

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            painter.setFont(font());
            painter.setPen(block == currentBlock ? activeColor : normalColor);
            painter.drawText(0, top, m_lineNumberArea->width() - 7, fontMetrics().height(),
                             Qt::AlignRight | Qt::AlignVCenter,
                             QString::number(blockNumber + 1));
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}
