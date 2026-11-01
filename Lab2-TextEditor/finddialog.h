#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include <QTextDocument>

QT_BEGIN_NAMESPACE
namespace Ui { class FindDialog; }
QT_END_NAMESPACE

class CodeEditor;

/**
 * 查找对话框：非模态，支持连续查找、回绕查找、大小写/全字匹配与全部高亮。
 */
class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(CodeEditor *editor, QWidget *parent = nullptr);
    ~FindDialog() override;

    // 由主窗口带入编辑器里当前选中的文字
    void setSearchText(const QString &text);
    QString searchText() const;

public slots:
    void findNext();
    void findPrevious();
    void refreshHighlights();

signals:
    void searchFinished(bool found);

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

    bool performSearch(bool backward);
    QTextDocument::FindFlags currentFlags(bool backward) const;
    void updateMatchInfo(const QTextCursor &found, bool wrapped);
    void applyHighlights();

    Ui::FindDialog *ui;
    CodeEditor *m_editor;

private:
    void handleFindTextChanged();
};

#endif // FINDDIALOG_H
