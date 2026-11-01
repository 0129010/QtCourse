#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "codeeditor.h"
#include "finddialog.h"
#include "replacedialog.h"

#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QColorDialog>
#include <QComboBox>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontComboBox>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QStatusBar>
#include <QTextCodec>
#include <QTextStream>
#include <QTime>
#include <QTimer>
#include <QToolBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_autoSaveTimer = new QTimer(this);
    m_autoSaveTimer->setTimerType(Qt::CoarseTimer);

    setAcceptDrops(true);
    setupExtraWidgets();
    setupConnections();

    ui->editor->setFocus();
    updateStatusBar();
    updateActionStates();
    updateWindowTitle();
    statusBar()->showMessage(tr("就绪"), 3000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ------------------------------------------------------------------ 初始化
void MainWindow::setupExtraWidgets()
{
    // 状态栏右侧的常驻信息：光标位置 / 字符数 / 自动保存状态
    m_positionLabel = new QLabel(tr("行 1，列 1"), this);
    m_charCountLabel = new QLabel(tr("字符数：0"), this);
    m_autoSaveLabel = new QLabel(tr("自动保存：关闭"), this);
    m_positionLabel->setMinimumWidth(110);
    m_charCountLabel->setMinimumWidth(130);
    m_autoSaveLabel->setMinimumWidth(150);
    ui->statusBar->addPermanentWidget(m_positionLabel);
    ui->statusBar->addPermanentWidget(m_charCountLabel);
    ui->statusBar->addPermanentWidget(m_autoSaveLabel);

    // 格式工具栏：字体、字号下拉框在设计器里不好直接放，用代码插到工具栏最前面
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
    connect(ui->actionFind, &QAction::triggered, this, &MainWindow::showFindDialog);
    connect(ui->actionReplace, &QAction::triggered, this, &MainWindow::showReplaceDialog);

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
    connect(ui->actionLineNumber, &QAction::toggled, this, &MainWindow::toggleLineNumber);
    connect(ui->actionAutoSave, &QAction::toggled, this, &MainWindow::toggleAutoSave);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &MainWindow::autoSaveOnTimeout);

    // 帮助
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAbout);
    connect(ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);

    // 状态同步：文本、光标、撤销栈、剪贴板任一变化都要刷新动作的使能状态
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
    if (!maybeSave())
        return;

    ui->editor->clear();
    ui->editor->document()->clearUndoRedoStacks();
    setCurrentFile(QString());
    ui->editor->setFocus();
    updateStatusBar();
    statusBar()->showMessage(tr("已新建空白文档"), 3000);
}

void MainWindow::open()
{
    if (!maybeSave())
        return;

    const QString filter = tr("文本文件 (*.txt *.md *.log *.cpp *.h *.json *.ini);;所有文件 (*.*)");
    const QString fileName = QFileDialog::getOpenFileName(this, tr("打开文件"),
                                                          QDir::homePath(), filter);
    if (fileName.isEmpty()) {
        statusBar()->showMessage(tr("已取消打开"), 2000);
        return;
    }

    loadFile(fileName);
}

bool MainWindow::loadFile(const QString &fileName)
{
    const QFileInfo info(fileName);

    // 边界：文件不存在（被删除、路径写错、U 盘已拔出）
    if (!info.exists()) {
        QMessageBox::warning(this, tr("打开文件"),
                             tr("文件不存在：\n%1\n\n请确认路径是否正确，或者文件是否已被移动、删除。")
                                 .arg(QDir::toNativeSeparators(fileName)));
        statusBar()->showMessage(tr("打开失败：文件不存在"), 4000);
        return false;
    }

    // 边界：选中的是目录或特殊设备
    if (!info.isFile()) {
        QMessageBox::warning(this, tr("打开文件"),
                             tr("“%1”不是普通文件，无法作为文本打开。").arg(info.fileName()));
        statusBar()->showMessage(tr("打开失败：不是普通文件"), 4000);
        return false;
    }

    // 边界：超大文件先确认，避免一次性读入造成界面长时间无响应
    const qint64 kMaxOpenSize = 8 * 1024 * 1024;
    if (info.size() > kMaxOpenSize) {
        const QString sizeText = QString::number(info.size() / 1024.0 / 1024.0, 'f', 1);
        const QMessageBox::StandardButton answer = QMessageBox::question(
            this, tr("打开文件"),
            tr("“%1”大小约 %2 MB，打开后界面可能变慢，是否继续？")
                .arg(info.fileName(), sizeText),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (answer != QMessageBox::Yes) {
            statusBar()->showMessage(tr("已取消打开大文件"), 3000);
            return false;
        }
    }

    QString content;
    QString message;
    bool usedFallbackCodec = false;
    bool wasEmpty = false;
    if (!readFileContent(fileName, &content, &message, &usedFallbackCodec, &wasEmpty)) {
        QMessageBox::warning(this, tr("打开文件"), message);
        statusBar()->showMessage(tr("打开失败：无法读取文件内容"), 4000);
        return false;
    }

    ui->editor->setPlainText(content);
    ui->editor->document()->clearUndoRedoStacks();
    ui->editor->moveCursor(QTextCursor::Start);
    setCurrentFile(fileName);
    ui->editor->setFocus();
    updateStatusBar();

    if (wasEmpty)
        statusBar()->showMessage(tr("“%1”是空文件").arg(info.fileName()), 4000);
    else if (usedFallbackCodec)
        statusBar()->showMessage(tr("文件不是 UTF-8 编码，已按 GB18030 解码打开"), 6000);
    else
        statusBar()->showMessage(tr("已打开 %1").arg(info.fileName()), 4000);

    return true;
}

bool MainWindow::readFileContent(const QString &fileName, QString *content, QString *message,
                                 bool *usedFallbackCodec, bool *wasEmpty)
{
    content->clear();
    message->clear();
    *usedFallbackCodec = false;
    *wasEmpty = false;

    QFile file(fileName);
    // 边界：没有读权限、文件被独占、路径是一个不可读的设备
    if (!file.open(QIODevice::ReadOnly)) {
        *message = tr("无法读取文件：\n%1\n\n原因：%2")
                       .arg(QDir::toNativeSeparators(fileName), file.errorString());
        return false;
    }

    const QByteArray raw = file.readAll();
    file.close();

    if (raw.isEmpty()) {
        *wasEmpty = true;
        return true;   // 空文件按空文档打开，不算失败
    }

    // 带 BOM 的文件优先按 BOM 解码
    if (raw.startsWith(QByteArray::fromHex("EFBBBF")))
        *content = QString::fromUtf8(raw.mid(3));
    else if (raw.startsWith(QByteArray::fromHex("FFFE")))
        *content = QTextCodec::codecForName("UTF-16LE")->toUnicode(raw.mid(2));
    else if (raw.startsWith(QByteArray::fromHex("FEFF")))
        *content = QTextCodec::codecForName("UTF-16BE")->toUnicode(raw.mid(2));
    else {
        // 没有 BOM：先严格按 UTF-8 解码，出现非法字节再退回 GB18030
        QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");
        QTextCodec::ConverterState state;
        const QString text = utf8->toUnicode(raw.constData(), raw.size(), &state);
        if (state.invalidChars > 0) {
            QTextCodec *gb18030 = QTextCodec::codecForName("GB18030");
            *content = gb18030 ? gb18030->toUnicode(raw) : QString::fromLocal8Bit(raw);
            *usedFallbackCodec = true;
        } else {
            *content = text;
        }
    }

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

    if (saveToFile(m_currentFilePath)) {
        statusBar()->showMessage(tr("已保存到 %1")
                                     .arg(QDir::toNativeSeparators(m_currentFilePath)), 4000);
        return true;
    }
    return false;
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
    if (dialog.exec() != QDialog::Accepted) {
        statusBar()->showMessage(tr("已取消另存为"), 2000);
        return false;
    }

    const QStringList selected = dialog.selectedFiles();
    if (selected.isEmpty())
        return false;

    QString fileName = selected.first();
    // 用户没写扩展名时补上 .txt，避免生成无后缀文件
    if (QFileInfo(fileName).suffix().isEmpty())
        fileName += QStringLiteral(".txt");

    if (!saveToFile(fileName))
        return false;

    statusBar()->showMessage(tr("已另存为 %1").arg(QDir::toNativeSeparators(fileName)), 4000);
    return true;
}

bool MainWindow::saveToFile(const QString &fileName, bool silent)
{
    QFile file(fileName);

    // 边界：目标只读、目录不存在、磁盘已满、无写权限
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        const QString reason = file.errorString();
        if (silent) {
            statusBar()->showMessage(tr("自动保存失败：%1").arg(reason), 6000);
            return false;
        }
        QMessageBox::warning(this, tr("保存文件"),
                             tr("无法写入文件：\n%1\n\n原因：%2\n\n内容仍保留在编辑器中，可以另存到其他位置。")
                                 .arg(QDir::toNativeSeparators(fileName), reason));
        statusBar()->showMessage(tr("保存失败"), 4000);
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << ui->editor->toPlainText();
    out.flush();

    const bool writeOk = (file.error() == QFileDevice::NoError);
    const QString errorText = file.errorString();
    file.close();

    if (!writeOk) {
        if (silent) {
            statusBar()->showMessage(tr("自动保存失败：%1").arg(errorText), 6000);
            return false;
        }
        QMessageBox::warning(this, tr("保存文件"),
                             tr("写入文件时出错：\n%1\n\n原因：%2")
                                 .arg(QDir::toNativeSeparators(fileName), errorText));
        return false;
    }

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
    if (!ui->editor->textCursor().hasSelection()) {
        statusBar()->showMessage(tr("没有选中内容，无法剪切"), 3000);
        return;
    }
    ui->editor->cut();
    updateActionStates();
}

void MainWindow::copy()
{
    if (!ui->editor->textCursor().hasSelection()) {
        statusBar()->showMessage(tr("没有选中内容，无法复制"), 3000);
        return;
    }
    ui->editor->copy();
    statusBar()->showMessage(tr("已复制选中内容"), 2000);
}

void MainWindow::paste()
{
    if (!QApplication::clipboard()->mimeData()->hasText()) {
        statusBar()->showMessage(tr("剪贴板中没有文本"), 3000);
        return;
    }
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
    ui->editor->refreshLineNumberWidth();
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
        cursor.select(QTextCursor::Document);   // 没有选中就调整整篇文档
    cursor.mergeCharFormat(format);

    QFont editorFont = ui->editor->font();
    editorFont.setPointSize(pointSize);
    ui->editor->document()->setDefaultFont(editorFont);
    ui->editor->setFont(editorFont);
    ui->editor->refreshLineNumberWidth();

    statusBar()->showMessage(tr("字号已设置为 %1 pt").arg(pointSize), 3000);
}

void MainWindow::chooseFontColor()
{
    const QColor initial = ui->editor->currentCharFormat().foreground().color();
    const QColor color = QColorDialog::getColor(initial, this, tr("选择字体颜色"));
    if (!color.isValid()) {
        statusBar()->showMessage(tr("已取消颜色选择"), 2000);
        return;
    }

    QTextCharFormat format;
    format.setForeground(color);
    ui->editor->mergeCurrentCharFormat(format);
    statusBar()->showMessage(tr("字体颜色已改为 %1").arg(color.name()), 3000);
}

void MainWindow::chooseBackgroundColor()
{
    const QColor initial = ui->editor->editorBackground();
    const QColor color = QColorDialog::getColor(initial, this, tr("选择背景颜色"));
    if (!color.isValid()) {
        statusBar()->showMessage(tr("已取消颜色选择"), 2000);
        return;
    }

    ui->editor->setEditorBackground(color);
    statusBar()->showMessage(tr("背景色已改为 %1").arg(color.name()), 3000);
}

void MainWindow::setWordWrap(bool enabled)
{
    ui->editor->setLineWrapMode(enabled ? QPlainTextEdit::WidgetWidth
                                        : QPlainTextEdit::NoWrap);
    ui->editor->refreshLineNumberWidth();
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
    if (visible)
        statusBar()->showMessage(tr("状态栏已显示"), 3000);
}

void MainWindow::toggleLineNumber(bool visible)
{
    ui->editor->setLineNumberVisible(visible);
    statusBar()->showMessage(visible ? tr("已显示行号") : tr("已隐藏行号"), 3000);
}

void MainWindow::toggleAutoSave(bool enabled)
{
    if (!enabled) {
        m_autoSaveTimer->stop();
        m_autoSaveLabel->setText(tr("自动保存：关闭"));
        statusBar()->showMessage(tr("已关闭定时自动保存"), 3000);
        return;
    }

    bool ok = false;
    const int seconds = QInputDialog::getInt(this, tr("定时自动保存"),
                                             tr("自动保存间隔（秒，10～600）："),
                                             m_autoSaveInterval, 10, 600, 5, &ok);
    if (!ok) {
        // 取消设置时把动作恢复为未选中，保证“菜单勾选状态”和实际行为一致
        ui->actionAutoSave->setChecked(false);
        return;
    }

    m_autoSaveInterval = seconds;
    m_autoSaveTimer->start(seconds * 1000);
    m_autoSaveLabel->setText(tr("自动保存：每 %1 秒").arg(seconds));
    statusBar()->showMessage(tr("已开启定时自动保存：每 %1 秒").arg(seconds), 4000);
}

void MainWindow::autoSaveOnTimeout()
{
    if (!ui->editor->document()->isModified())
        return;

    if (m_currentFilePath.isEmpty()) {
        statusBar()->showMessage(tr("自动保存已跳过：文件还没有保存到磁盘，请先手动保存"), 5000);
        return;
    }

    // silent 模式：自动保存不弹对话框，避免用户正在输入时被打断
    if (saveToFile(m_currentFilePath, true)) {
        m_autoSaveLabel->setText(tr("自动保存：%1 已保存")
                                     .arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss"))));
    } else {
        m_autoSaveTimer->stop();
        ui->actionAutoSave->setChecked(false);
        m_autoSaveLabel->setText(tr("自动保存：写入失败，已停止"));
        statusBar()->showMessage(tr("自动保存失败，已停止定时器，请检查文件权限"), 8000);
    }
}

// ------------------------------------------------------------------ 查找替换
void MainWindow::showFindDialog()
{
    if (!m_findDialog) {
        m_findDialog = new FindDialog(ui->editor, this);
        connect(m_findDialog, &FindDialog::searchFinished, this, [this](bool found) {
            statusBar()->showMessage(found ? tr("查找成功") : tr("没有找到匹配内容"), 2500);
        });
    }

    m_findDialog->setSearchText(ui->editor->textCursor().selectedText());
    m_findDialog->show();
    m_findDialog->raise();
    m_findDialog->activateWindow();
}

void MainWindow::showReplaceDialog()
{
    if (!m_replaceDialog) {
        m_replaceDialog = new ReplaceDialog(ui->editor, this);
    }

    m_replaceDialog->setSearchText(ui->editor->textCursor().selectedText());
    m_replaceDialog->show();
    m_replaceDialog->raise();
    m_replaceDialog->activateWindow();
}

// ------------------------------------------------------------------ 帮助
void MainWindow::showAbout()
{
    QMessageBox::about(
        this, tr("关于 简易文本编辑器"),
        tr("<h3>简易文本编辑器 v1.0</h3>"
           "<p>《Qt 应用程序开发》实验二 · 多窗口文本编辑器</p>"
           "<p>基于 Qt %1 与 QMainWindow / QPlainTextEdit 实现。</p>"
           "<p><b>基础功能：</b>文件新建/打开/保存/另存为，撤销重做与剪贴板操作，"
           "字体、字号、字体颜色、背景色，自动换行，窗口标题随修改状态自动更新。</p>"
           "<p><b>扩展功能：</b>行号栏（CodeEditor 派生类）、查找与替换（连续查找、回绕查找、"
           "全部高亮、全部替换）、编码自动识别（UTF-8/UTF-16/GB18030）、大文件保护、"
           "未保存提示、定时自动保存、拖放打开。</p>"
           "<p>东莞理工学院 · 计算机科学与技术学院</p>")
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

    // 撤销/重做看文档撤销栈是否为空
    ui->actionUndo->setEnabled(document->isUndoAvailable());
    ui->actionRedo->setEnabled(document->isRedoAvailable());

    // 剪切/复制依赖是否有选中内容
    ui->actionCut->setEnabled(hasSelection);
    ui->actionCopy->setEnabled(hasSelection);

    // 粘贴依赖剪贴板里是否有文本
    const QMimeData *mime = QApplication::clipboard()->mimeData();
    ui->actionPaste->setEnabled(mime != nullptr && mime->hasText());

    // 空文档没有可全选的内容
    ui->actionSelectAll->setEnabled(!document->isEmpty());
}

void MainWindow::updateWindowTitle()
{
    setWindowModified(ui->editor->document()->isModified());
    // [*] 是 Qt 的占位符：文档被修改后会自动显示成 *
    setWindowTitle(tr("%1[*] - 简易文本编辑器").arg(displayName()));
}

// ------------------------------------------------------------------ 关闭与拖放
bool MainWindow::maybeSave()
{
    if (!ui->editor->document()->isModified())
        return true;

    QMessageBox box(this);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle(tr("简易文本编辑器"));
    box.setText(tr("文档“%1”有未保存的修改。").arg(displayName()));
    box.setInformativeText(tr("是否先保存修改，再继续操作？"));

    QPushButton *saveButton = box.addButton(tr("保存"), QMessageBox::AcceptRole);
    QPushButton *discardButton = box.addButton(tr("不保存"), QMessageBox::DestructiveRole);
    QPushButton *cancelButton = box.addButton(tr("取消"), QMessageBox::RejectRole);
    saveButton->setObjectName(QStringLiteral("buttonSaveDocument"));
    discardButton->setObjectName(QStringLiteral("buttonDiscardDocument"));
    cancelButton->setObjectName(QStringLiteral("buttonCancelClose"));
    box.setDefaultButton(saveButton);
    box.setEscapeButton(cancelButton);
    box.exec();

    if (box.clickedButton() == saveButton)
        return doSave();          // 保存失败或取消另存为时返回 false，不关闭窗口
    if (box.clickedButton() == discardButton)
        return true;              // 放弃修改
    return false;                 // 取消：留在当前文档继续编辑
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!maybeSave()) {
        event->ignore();
        return;
    }

    if (m_findDialog)
        m_findDialog->close();
    if (m_replaceDialog)
        m_replaceDialog->close();
    m_autoSaveTimer->stop();

    event->accept();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *mime = event->mimeData();
    if (mime && mime->hasUrls()) {
        const QList<QUrl> urls = mime->urls();
        if (!urls.isEmpty() && urls.first().isLocalFile()) {
            event->acceptProposedAction();
            return;
        }
    }
    event->ignore();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *mime = event->mimeData();
    if (!mime || !mime->hasUrls()) {
        event->ignore();
        return;
    }

    const QList<QUrl> urls = mime->urls();
    if (urls.isEmpty() || !urls.first().isLocalFile()) {
        event->ignore();
        return;
    }

    if (!maybeSave()) {
        event->ignore();
        return;
    }

    if (loadFile(urls.first().toLocalFile()))
        event->acceptProposedAction();
    else
        event->ignore();
}
