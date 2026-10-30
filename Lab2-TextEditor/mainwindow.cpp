#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QApplication>
#include <QClipboard>
#include <QColorDialog>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontComboBox>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QPlainTextEdit>
#include <QStatusBar>
#include <QTextStream>
#include <QToolBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupExtraWidgets();
    setupConnections();

    ui->editor->setFocus();
    updateStatusBar();
    updateActionStates();
    updateWindowTitle();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ------------------------------------------------------------------ 初始化
void MainWindow::setupExtraWidgets()
{
    m_positionLabel = new QLabel(tr("行 1，列 1"), this);
    m_charCountLabel = new QLabel(tr("字符数：0"), this);
    m_positionLabel->setMinimumWidth(110);
    m_charCountLabel->setMinimumWidth(130);
    ui->statusBar->addPermanentWidget(m_positionLabel);
    ui->statusBar->addPermanentWidget(m_charCountLabel);

    m_fontFamilyBox = new QFontComboBox(ui->toolBarFormat);
    m_fontFamilyBox->setToolTip(tr("字体"));
    m_fontFamilyBox->setMinimumWidth(170);
    m_fontFamilyBox->setCurrentFont(ui->editor->font());

    m_fontSizeBox = new QComboBox(ui->toolBarFormat);
    m_fontSizeBox->setToolTip(tr("字号"));
    m_fontSizeBox->setEditable(true);
    m_fontSizeBox->setMinimumWidth(64);
    m_fontSizeBox->setMaximumWidth(84);
    const int sizes[] = {9, 10, 11, 12, 14, 16, 18, 20, 24, 28, 32, 36, 48};
    for (int size : sizes)
        m_fontSizeBox->addItem(QString::number(size));
    m_fontSizeBox->setCurrentText(QString::number(ui->editor->font().pointSize()));

    QAction *separator = ui->toolBarFormat->insertSeparator(ui->actionFontColor);
    ui->toolBarFormat->insertWidget(separator, m_fontFamilyBox);
    ui->toolBarFormat->insertWidget(separator, m_fontSizeBox);
}

void MainWindow::setupConnections()
{
    // 文件
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::open);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::save);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveAs);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);

    // 编辑
    connect(ui->actionUndo, &QAction::triggered, this, &MainWindow::undo);
    connect(ui->actionRedo, &QAction::triggered, this, &MainWindow::redo);
    connect(ui->actionCut, &QAction::triggered, this, &MainWindow::cut);
    connect(ui->actionCopy, &QAction::triggered, this, &MainWindow::copy);
    connect(ui->actionPaste, &QAction::triggered, this, &MainWindow::paste);
    connect(ui->actionSelectAll, &QAction::triggered, this, &MainWindow::selectAll);

    // 格式
    connect(m_fontFamilyBox, &QFontComboBox::currentFontChanged,
            this, &MainWindow::setFontFamily);
    connect(m_fontSizeBox, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        setFontSize(text.toInt());
    });
    connect(ui->actionFontColor, &QAction::triggered, this, &MainWindow::chooseFontColor);
    connect(ui->actionBackgroundColor, &QAction::triggered,
            this, &MainWindow::chooseBackgroundColor);
    connect(ui->actionWordWrap, &QAction::toggled, this, &MainWindow::setWordWrap);

    // 视图
    connect(ui->actionToggleToolBar, &QAction::toggled, this, &MainWindow::toggleToolBars);
    connect(ui->actionToggleStatusBar, &QAction::toggled, this, &MainWindow::toggleStatusBar);

    // 帮助
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAbout);
    connect(ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);

    // 状态同步
    connect(ui->editor, &QPlainTextEdit::textChanged, this, &MainWindow::updateStatusBar);
    connect(ui->editor, &QPlainTextEdit::textChanged, this, &MainWindow::updateActionStates);
    connect(ui->editor, &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::updateStatusBar);
    connect(ui->editor, &QPlainTextEdit::cursorPositionChanged,
            this, &MainWindow::updateActionStates);
    connect(ui->editor, &QPlainTextEdit::copyAvailable, this, &MainWindow::updateActionStates);
    connect(ui->editor->document(), &QTextDocument::modificationChanged,
            this, &MainWindow::updateWindowTitle);
    connect(ui->editor->document(), &QTextDocument::undoAvailable,
            this, &MainWindow::updateActionStates);
    connect(ui->editor->document(), &QTextDocument::redoAvailable,
            this, &MainWindow::updateActionStates);
    connect(QApplication::clipboard(), &QClipboard::dataChanged,
            this, &MainWindow::updateActionStates);
}

// ------------------------------------------------------------------ 文件操作
void MainWindow::newFile()
{
    ui->editor->clear();
    ui->editor->document()->clearUndoRedoStacks();
    setCurrentFile(QString());
    ui->editor->setFocus();
    updateStatusBar();
    statusBar()->showMessage(tr("已新建空白文档"), 3000);
}

void MainWindow::open()
{
    const QString filter = tr("文本文件 (*.txt *.md *.log *.cpp *.h *.json *.ini);;所有文件 (*.*)");
    const QString fileName = QFileDialog::getOpenFileName(this, tr("打开文件"),
                                                          QDir::homePath(), filter);
    if (fileName.isEmpty())
        return;

    loadFile(fileName);
}

bool MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("打开文件"),
                             tr("无法打开文件：\n%1\n\n原因：%2")
                                 .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    ui->editor->setPlainText(in.readAll());
    file.close();

    ui->editor->document()->clearUndoRedoStacks();
    ui->editor->moveCursor(QTextCursor::Start);
    setCurrentFile(fileName);
    ui->editor->setFocus();
    updateStatusBar();
    statusBar()->showMessage(tr("已打开 %1").arg(QFileInfo(fileName).fileName()), 4000);
    return true;
}

void MainWindow::save()
{
    doSave();
}

bool MainWindow::doSave()
{
    if (m_currentFilePath.isEmpty())
        return doSaveAs();
    return saveToFile(m_currentFilePath);
}

void MainWindow::saveAs()
{
    doSaveAs();
}

bool MainWindow::doSaveAs()
{
    QString startPath = m_currentFilePath;
    if (startPath.isEmpty())
        startPath = QDir::homePath() + QDir::separator() + tr("未命名.txt");

    QFileDialog dialog(this, tr("另存为"), startPath,
                       tr("文本文件 (*.txt);;Markdown 文件 (*.md);;所有文件 (*.*)"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix(QStringLiteral("txt"));
    if (dialog.exec() != QDialog::Accepted)
        return false;

    const QStringList selected = dialog.selectedFiles();
    if (selected.isEmpty())
        return false;

    QString fileName = selected.first();
    if (QFileInfo(fileName).suffix().isEmpty())
        fileName += QStringLiteral(".txt");

    return saveToFile(fileName);
}

bool MainWindow::saveToFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this, tr("保存文件"),
                             tr("无法写入文件：\n%1\n\n原因：%2")
                                 .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << ui->editor->toPlainText();
    out.flush();
    file.close();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("已保存到 %1").arg(QDir::toNativeSeparators(fileName)), 4000);
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    m_currentFilePath = fileName;
    ui->editor->document()->setModified(false);
    updateWindowTitle();
    updateActionStates();
}

QString MainWindow::displayName() const
{
    if (m_currentFilePath.isEmpty())
        return tr("未命名");
    return QFileInfo(m_currentFilePath).fileName();
}

// ------------------------------------------------------------------ 编辑操作
void MainWindow::undo()
{
    ui->editor->undo();
    updateActionStates();
}

void MainWindow::redo()
{
    ui->editor->redo();
    updateActionStates();
}

void MainWindow::cut()
{
    ui->editor->cut();
    updateActionStates();
}

void MainWindow::copy()
{
    ui->editor->copy();
    statusBar()->showMessage(tr("已复制选中内容"), 2000);
}

void MainWindow::paste()
{
    ui->editor->paste();
    updateActionStates();
}

void MainWindow::selectAll()
{
    ui->editor->selectAll();
    updateStatusBar();
}

// ------------------------------------------------------------------ 格式
void MainWindow::setFontFamily(const QFont &font)
{
    QFont editorFont = ui->editor->font();
    editorFont.setFamily(font.family());
    ui->editor->setFont(editorFont);
    ui->editor->document()->setDefaultFont(editorFont);
    statusBar()->showMessage(tr("字体已切换为 %1").arg(font.family()), 3000);
}

void MainWindow::setFontSize(int pointSize)
{
    if (pointSize <= 0 || pointSize > 200) {
        statusBar()->showMessage(tr("字号必须是 1～200 之间的整数"), 3000);
        return;
    }

    QTextCharFormat format;
    format.setFontPointSize(pointSize);

    QTextCursor cursor = ui->editor->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::Document);
    cursor.mergeCharFormat(format);

    QFont editorFont = ui->editor->font();
    editorFont.setPointSize(pointSize);
    ui->editor->document()->setDefaultFont(editorFont);
    ui->editor->setFont(editorFont);

    statusBar()->showMessage(tr("字号已设置为 %1 pt").arg(pointSize), 3000);
}

void MainWindow::chooseFontColor()
{
    const QColor initial = ui->editor->currentCharFormat().foreground().color();
    const QColor color = QColorDialog::getColor(initial, this, tr("选择字体颜色"));
    if (!color.isValid())
        return;

    QTextCharFormat format;
    format.setForeground(color);
    ui->editor->mergeCurrentCharFormat(format);
    statusBar()->showMessage(tr("字体颜色已改为 %1").arg(color.name()), 3000);
}

void MainWindow::chooseBackgroundColor()
{
    QPalette palette = ui->editor->palette();
    const QColor color = QColorDialog::getColor(palette.color(QPalette::Base), this,
                                                tr("选择背景颜色"));
    if (!color.isValid())
        return;

    palette.setColor(QPalette::Base, color);
    ui->editor->setPalette(palette);
    statusBar()->showMessage(tr("背景色已改为 %1").arg(color.name()), 3000);
}

void MainWindow::setWordWrap(bool enabled)
{
    ui->editor->setLineWrapMode(enabled ? QPlainTextEdit::WidgetWidth
                                        : QPlainTextEdit::NoWrap);
    statusBar()->showMessage(enabled ? tr("已开启自动换行") : tr("已关闭自动换行"), 3000);
}

// ------------------------------------------------------------------ 视图
void MainWindow::toggleToolBars(bool visible)
{
    const QList<QToolBar *> bars = findChildren<QToolBar *>();
    for (QToolBar *bar : bars)
        bar->setVisible(visible);
    statusBar()->showMessage(visible ? tr("工具栏已显示") : tr("工具栏已隐藏"), 3000);
}

void MainWindow::toggleStatusBar(bool visible)
{
    ui->statusBar->setVisible(visible);
}

// ------------------------------------------------------------------ 帮助
void MainWindow::showAbout()
{
    QMessageBox::about(this, tr("关于 简单文本编辑器"),
                       tr("<h3>简单文本编辑器 v0.1</h3>"
                          "<p>《Qt 应用程序开发》实验二</p>"
                          "<p>基于 Qt %1 与 QMainWindow / QPlainTextEdit 实现。</p>"
                          "<p>已实现：文件的打开与保存、撤销重做、剪贴板操作、字体与颜色设置、"
                          "自动换行、工具栏与状态栏切换。</p>")
                           .arg(QString::fromLatin1(QT_VERSION_STR)));
}

// ------------------------------------------------------------------ 状态同步
void MainWindow::updateStatusBar()
{
    const QTextCursor cursor = ui->editor->textCursor();
    const int line = cursor.blockNumber() + 1;
    const int column = cursor.positionInBlock() + 1;
    m_positionLabel->setText(tr("行 %1，列 %2").arg(line).arg(column));

    const int total = qMax(0, ui->editor->document()->characterCount() - 1);
    if (cursor.hasSelection())
        m_charCountLabel->setText(tr("选中 %1 / 共 %2 字符")
                                      .arg(cursor.selectionEnd() - cursor.selectionStart())
                                      .arg(total));
    else
        m_charCountLabel->setText(tr("字符数：%1").arg(total));
}

void MainWindow::updateActionStates()
{
    QTextDocument *document = ui->editor->document();
    const bool hasSelection = ui->editor->textCursor().hasSelection();

    ui->actionUndo->setEnabled(document->isUndoAvailable());
    ui->actionRedo->setEnabled(document->isRedoAvailable());
    ui->actionCut->setEnabled(hasSelection);
    ui->actionCopy->setEnabled(hasSelection);

    const QMimeData *mime = QApplication::clipboard()->mimeData();
    ui->actionPaste->setEnabled(mime != nullptr && mime->hasText());
    ui->actionSelectAll->setEnabled(!document->isEmpty());
}

void MainWindow::updateWindowTitle()
{
    setWindowModified(ui->editor->document()->isModified());
    setWindowTitle(tr("%1[*] - 简单文本编辑器").arg(displayName()));
}
