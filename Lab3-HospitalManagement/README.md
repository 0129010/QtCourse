# 实验三：QtSQL 数据库模块与 View-Model 结构的使用

## 一、项目简介

本项目是《Qt 应用程序开发》实验三的提交代码，用 Qt 的 SQL 模块 + View/Model 结构实现一个
“医院诊疗系统 - 用户信息管理”桌面应用。程序启动后先弹出登录窗口，登录成功后进入主窗口，
可以在患者、医生、科室三个维度维护数据，并查看用户操作记录。

数据持久化使用 SQLite（Qt 自带的 `QSQLITE` 驱动），数据库文件 `clinic.db` 在程序首次运行时
自动创建于可执行文件所在目录，并写入演示数据。

## 二、功能清单

| 模块 | 功能 | 使用的类 |
| --- | --- | --- |
| 登录 | 用户名/密码校验，失败给出红色提示并清空密码框 | `QDialog` + `DatabaseManager::login()` |
| 患者管理 | 列表显示、关键字查询、添加、修改、删除、刷新 | `PatientTableModel`（继承 `QSqlTableModel`）+ `QTableView` |
| 医生管理（选做） | 列表显示、添加、删除，科室列用下拉框切换 | `QSqlTableModel` + `ComboBoxDelegate` |
| 科室管理（选做） | 科室添加、删除（删除前检查是否有医生引用） | `QSqlTableModel` + `QSqlQuery` |
| 操作记录（选做） | 关联 `user` 表按时间倒序显示谁做了什么 | `QSqlQueryModel` |

界面上的主窗口状态栏固定显示「学号：2024414300103　姓名：高俊」，登录后还会显示当前用户、
数据库文件路径。

## 三、目录结构

```
Lab3-HospitalManagement/
├── Lab3.pro                     # qmake 工程文件（QT += sql widgets）
├── main.cpp                     # 程序入口 + 自动截图模式
├── db/
│   ├── databasemanager.h/.cpp   # 数据库单例：连接、建表、种子数据、登录、操作日志、主键生成
│   └── patienttablemodel.h/.cpp # 患者表模型：性别字段 1/0 ↔ 男/女 的显示映射
├── ui/
│   ├── logindialog.h/.cpp       # 登录窗口
│   ├── mainwindow.h/.cpp        # 主窗口：工具栏 + QStackedWidget 四页切换 + 状态栏
│   ├── patientpage.h/.cpp       # 患者管理页（查询 + 增删改）
│   ├── patientdialog.h/.cpp     # 添加/修改患者信息的表单对话框与输入校验
│   ├── doctorpage.h/.cpp        # 医生管理页（选做）
│   ├── departmentpage.h/.cpp    # 科室管理页（选做）
│   ├── historypage.h/.cpp       # 操作记录页（选做）
│   └── delegate/
│       ├── comboboxdelegate.*   # 下拉框代理（性别、科室）
│       └── doublespindelegate.* # 数值框代理（身高、体重）
└── util/
    └── messagebox.h/.cpp        # 统一封装的提示/确认对话框
```

## 四、MVC（View-Model）结构说明

- **Model（数据层）**：`db/databasemanager` 负责连接与建表；`QSqlTableModel` /
  `PatientTableModel` 把表数据包装成模型，`QSqlQueryModel` 用于只读的联表查询。
- **View（视图层）**：`QTableView`、`QDialog` 只负责展示与收集输入，本身不写 SQL。
- **Controller（控制层）**：`PatientPage` / `DoctorPage` 等页面类响应按钮，调用 Model 的
  `setFilter()`、`select()` 或 `QSqlQuery` 完成增删改，再把结果刷新回 View。

关键细节：表格里编辑性别、身高、体重时，写回数据库的值由代理（delegate）与模型共同保证类型一致
（见 `ui/delegate/` 与 `db/patienttablemodel.cpp`）。

## 五、构建与运行

环境：Qt 5.12.12 (MinGW 7.3 64-bit)、Qt Creator。

```bash
# 命令行构建
qmake Lab3.pro
mingw32-make -j4
./release/Lab3.exe
```

也可以在 Qt Creator 中直接打开 `Lab3.pro` 后构建运行。若用命令行运行，需保证 Qt 的 bin 目录
在 `PATH` 中（或使用 `windeployqt` 拷贝依赖 DLL）。

## 六、演示账号

| 用户名 | 密码 | 说明 |
| --- | --- | --- |
| admin | 123456 | 系统管理员 |
| doctor | 123456 | 门诊医生 |

## 七、自动截图模式（用于实验报告）

```bash
Lab3.exe --shots <输出目录>
```

程序会依次渲染登录页、登录失败提示、数据库连接失败提示、患者列表、查询命中/无结果、
添加对话框、必填校验提示、删除确认、医生页、科室页、操作记录页，并保存为 PNG。
之所以做成程序内 `QWidget::grab()`，是因为实验环境没有可用的 GUI 自动化工具，
这样既能保证截图与真实界面完全一致，也便于重复生成。
