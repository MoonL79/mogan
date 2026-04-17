# 模块 01: 启动性能

## 关键热点列表

- 启动顺序在显示启动登录窗之前就执行 `init_texmacs_front`、`load_settings_and_check_version`、`init_plugins`，其中包含路径建立、用户目录创建、环境变量拼装、插件初始化；这条链路在首屏前同步完成，见 `src/Mogan/Research/research.cpp:249-253` 与 `src/System/Boot/init_texmacs.cpp:567-575`。
- `init_user_dirs` 在启动时无条件创建大量目录并清理临时目录，`clean_temp_dirs` 还会遍历临时目录并对每个 pid 调 `ps`，首次启动和慢盘环境下都容易拉长冷启动，见 `src/System/Boot/init_texmacs.cpp:318-391`。
- 启动登录窗中的 `BootstrapTaskExecutor` 虽然分段，但所有步骤仍在主线程执行；特别是 `font_database_load` 和 `init_std_drd` 只是被 `QTimer` 切片，没有脱离 UI 线程，见 `src/Plugins/Qt/qt_guide_task_executor.cpp:116-170`、`200-239`。
- `research.cpp` 在 Qt 应用创建前后都提前读取用户偏好，`init_texmacs()` 内又再次调用 `load_user_preferences()`，存在重复启动初始化，见 `src/Mogan/Research/research.cpp:192-204` 与 `src/System/Boot/init_texmacs.cpp:577-592`。

## 可验证假设

- 给 `init_texmacs_front`、`init_plugins`、`font_database_load`、`init_std_drd` 增加阶段耗时埋点后，启动总时长会集中在这几个同步段。
- 冷启动慢于热启动的主要差异将集中在目录创建/扫描、字体数据库加载和插件相关路径拼装。
- 去掉启动窗本地事件循环后的主观流畅度未必明显改善，因为真正的耗时仍在主线程同步初始化。

## 优先级建议

- `P0`: `init_texmacs_front` 前半段和 `BootstrapTaskExecutor` 的主线程初始化。
- `P1`: `init_plugins`、设置读取重复路径。
- `P2`: 启动登录窗本身的 UI 组织方式。
