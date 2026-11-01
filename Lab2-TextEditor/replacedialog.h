#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>
#include <QTextDocument>

QT_BEGIN_NAMESPACE
namespace Ui { class ReplaceDialog; }
QT_END_NAMESPACE

class CodeEditor;

/**
 * 替换对话框：支持“替换当前一处”与“全部替换”，并对空关键字、未找到、
 * 替换串包含查找串等边界情况做了保护。
 */
class ReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReplaceDialog(CodeEditor *editor, QWidget *parent = nullptr);
    ~ReplaceDialog() override;

    void setSearchText(const QString &text);

public slots:
    void findNext();
    void replaceOne();
    void replaceAll();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    bool performSearch(bool backward);
    QTextDocument::FindFlags currentFlags(bool backward) const;
    void applyHighlights();
    void handleFindTextChanged();

    Ui::ReplaceDialog *ui;
    CodeEditor *m_editor;
};

#endif // REPLACEDIALOG_H
