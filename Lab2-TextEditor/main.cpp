#include "mainwindow.h"

#include <QApplication>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("简易文本编辑器"));
    app.setApplicationVersion(QStringLiteral("1.0"));
    app.setOrganizationName(QStringLiteral("DGUT"));

    // 加载 Qt 自带的中文翻译，让对话框里的标准按钮（确定/取消/是/否）也显示为中文
    QTranslator qtTranslator;
    if (qtTranslator.load(QStringLiteral("qt_zh_CN"),
                          QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        app.installTranslator(&qtTranslator);
    }

    MainWindow window;
    window.show();

    // 命令行（或在资源管理器里“用本程序打开”）传入路径时直接加载
    if (argc > 1) {
        const QString path = QString::fromLocal8Bit(argv[1]);
        if (QFileInfo::exists(path))
            window.loadFile(path);
    }

    return app.exec();
}
