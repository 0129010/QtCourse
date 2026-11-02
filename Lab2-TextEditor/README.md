# 简易文本编辑器（Mynotepad）

《Qt 应用程序开发》实验二 —— 多窗口文本编辑器。界面用 Qt Designer 搭建，菜单与工具栏全部由
`QAction` 驱动；编辑器控件 `CodeEditor` 由 `QPlainTextEdit` 派生，带行号栏与当前行高亮。

## 功能

**基础功能**

- 菜单栏 / 工具栏 / 状态栏三段式布局，Action 按“文件、编辑、格式、视图、帮助”分组
- 文件新建、打开、保存、另存为，窗口标题随修改状态自动显示 `*`
- 撤销、重做、剪切、复制、粘贴、全选
- 字体、字号、字体颜色、编辑器背景色
- 自动换行开关、行号开关、工具栏 / 状态栏显示开关
- 关于对话框

**扩展功能**

- 行号栏：`CodeEditor` 继承 `QPlainTextEdit`，在 Designer 中用 promote 提升，行号随滚动、换行、
  字号变化自动重排
- 查找：连续查找、回绕查找、大小写 / 全字匹配、全部匹配高亮、命中计数（第 n 处 / 共 m 处）
- 替换：单处替换、全部替换（替换串包含查找串时也不会死循环，并有单次替换上限保护）
- 编码识别：UTF-8（含 BOM）、UTF-16 LE/BE、GB18030 自动回退，保存统一 UTF-8
- 异常处理：文件不存在、无读权限、目录、> 8 MB 大文件、只读文件保存失败都有明确提示，
  且不破坏编辑器里的内容
- 未保存提示（保存 / 不保存 / 取消），取消时窗口不关闭
- 定时自动保存：可设置间隔（10～600 秒），仅在“已修改且已有磁盘路径”时写回；写入失败自动停表
- 拖放打开文件、命令行参数打开文件

## 编译

```bash
mkdir build && cd build
qmake ../Mynotepad.pro -spec win32-g++ "CONFIG+=release"
mingw32-make -j8
```

环境：Qt 5.12.12（MinGW 64 bit）+ Git。生成的 `release/Mynotepad.exe` 需要 Qt 的运行库，
用 `windeployqt` 拷贝一份即可分发。

## 目录结构

```
Mynotepad.pro          工程文件
main.cpp               程序入口
mainwindow.ui          Designer 界面（菜单、工具栏、Action、promote 后的 CodeEditor）
mainwindow.h/.cpp      主窗口逻辑：文件、编辑、格式、视图、状态同步
codeeditor.h/.cpp      带行号栏的编辑器控件（LineNumberArea）
finddialog.ui/.h/.cpp  查找对话框
replacedialog.ui/.h/.cpp 替换对话框
icons/                 工具栏 SVG 图标
resources.qrc          图标资源
docs/demo/             演示用文本
docs/screenshots/      运行截图（raw/ 为原始截图）
testdata/              边界测试时生成的临时文件（已 gitignore）
```

## 说明

- 保存一律使用 UTF-8（无 BOM），避免跨平台打开中文乱码；
- 打开文件时先按 UTF-8 严格解码，遇到非法字节再回退到 GB18030，并在状态栏提示；
- 自动保存使用静默模式，不弹对话框，写入失败会停止定时器并提示，避免反复打断输入。
