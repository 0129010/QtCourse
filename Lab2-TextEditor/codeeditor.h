#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QColor>
#include <QList>
#include <QPlainTextEdit>

class LineNumberArea;

/**
 * 带行号栏的文本编辑器。
 * 由 QPlainTextEdit 派生，左侧自绘行号，另负责当前行高亮与查找高亮的叠加。
 */
class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);

    int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent *event);

    bool isLineNumberVisible() const { return m_lineNumberVisible; }
    void setLineNumberVisible(bool visible);

    // 字号/字体改变或换行模式改变后，重新计算行号栏宽度
    void refreshLineNumberWidth();

    void setEditorBackground(const QColor &color);
    QColor editorBackground() const;

    // 查找高亮与当前行高亮共用 extraSelections，需要统一管理
    void setSearchHighlights(const QList<QTextEdit::ExtraSelection> &selections);
    void clearSearchHighlights();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);
    void highlightCurrentLine();

private:
    void updateExtraSelections();

    LineNumberArea *m_lineNumberArea;
    bool m_lineNumberVisible;
    QColor m_currentLineColor;
    QList<QTextEdit::ExtraSelection> m_cursorLineSelection;
    QList<QTextEdit::ExtraSelection> m_searchSelections;
};

#endif // CODEEDITOR_H
