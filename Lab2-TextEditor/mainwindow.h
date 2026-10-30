#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFont>
#include <QMainWindow>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QComboBox;
class QFontComboBox;
class QLabel;

/**
 * 主窗口：菜单栏、工具栏、编辑器、状态栏的组装，以及文件/编辑/格式/视图功能。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    bool loadFile(const QString &fileName);
    QString currentFilePath() const { return m_currentFilePath; }

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
    // 帮助
    void showAbout();
    // 状态同步
    void updateStatusBar();
    void updateActionStates();
    void updateWindowTitle();

private:
    void setupExtraWidgets();
    void setupConnections();
    bool doSave();
    bool doSaveAs();
    bool saveToFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    QString displayName() const;

    Ui::MainWindow *ui;
    QString m_currentFilePath;
    QComboBox *m_fontSizeBox = nullptr;
    QFontComboBox *m_fontFamilyBox = nullptr;
    QLabel *m_positionLabel = nullptr;
    QLabel *m_charCountLabel = nullptr;
};

#endif // MAINWINDOW_H
