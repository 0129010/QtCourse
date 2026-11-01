#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFont>
#include <QMainWindow>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class CodeEditor;
class FindDialog;
class ReplaceDialog;
class QComboBox;
class QFontComboBox;
class QLabel;
class QTimer;

/**
 * 主窗口：菜单栏、工具栏、编辑器、状态栏的组装，以及文件/编辑/格式/视图各功能的实现。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // “打开”动作、拖放与命令行参数共用这一个入口
    bool loadFile(const QString &fileName);
    QString currentFilePath() const { return m_currentFilePath; }

protected:
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    // 文件
    void newFile();
    void open();
    void save();
    void saveAs();
    // 编辑
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void selectAll();
    // 格式
    void setFontFamily(const QFont &font);
    void setFontSize(int pointSize);
    void chooseFontColor();
    void chooseBackgroundColor();
    void setWordWrap(bool enabled);
    // 视图
    void toggleToolBars(bool visible);
    void toggleStatusBar(bool visible);
    void toggleLineNumber(bool visible);
    void toggleAutoSave(bool enabled);
    void autoSaveOnTimeout();
    // 查找替换
    void showFindDialog();
    void showReplaceDialog();
    // 帮助
    void showAbout();
    // 状态同步
    void updateStatusBar();
    void updateActionStates();
    void updateWindowTitle();

private:
    void setupExtraWidgets();
    void setupConnections();
    bool maybeSave();
    bool doSave();
    bool doSaveAs();
    bool saveToFile(const QString &fileName, bool silent = false);
    void setCurrentFile(const QString &fileName);
    QString displayName() const;
    bool readFileContent(const QString &fileName, QString *content, QString *message,
                         bool *usedFallbackCodec, bool *wasEmpty);

    Ui::MainWindow *ui;
    QString m_currentFilePath;
    QComboBox *m_fontSizeBox = nullptr;
    QFontComboBox *m_fontFamilyBox = nullptr;
    QLabel *m_positionLabel = nullptr;
    QLabel *m_charCountLabel = nullptr;
    QLabel *m_autoSaveLabel = nullptr;
    QTimer *m_autoSaveTimer = nullptr;
    FindDialog *m_findDialog = nullptr;
    ReplaceDialog *m_replaceDialog = nullptr;
    int m_autoSaveInterval = 30;
};

#endif // MAINWINDOW_H
