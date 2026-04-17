# 模块 09: Qt/UI 层

## 关键热点列表

- 启动页文件面板 `showEvent` 里立即刷新最近文档并触发布局重排；`resizeEvent` 宽度变化时全量重排卡片，见 `src/Plugins/Qt/qt_file_page.cpp:228-248`、`273-295`。
- 模板页首次进入会搭建大批按钮、卡片和滚动布局，后续分类栏也采用先清空再重建策略，见 `src/Plugins/Qt/qt_template_page.cpp:133-239`。
- 主窗口启动阶段同时创建状态栏、提示条、登录按钮、更新提示与定时任务，用户可见层组件初始化偏重，见 `src/Plugins/Qt/qt_tm_widget.cpp:360-456`。
- `sync_startup_tab_mode()` 会在 startup tab 与正常视图之间切换内容 widget，若频繁触发，容易带来多余 show/hide 与布局刷新，见 `src/Plugins/Qt/qt_tm_widget.cpp:870-896`。

## 可验证假设

- 启动页而非编辑页本身，可能就是“第一印象慢”的重要来源之一。
- 启动页切换、窗口 resize、模板分类切换会比预期触发更多布局计算。
- UI 体感慢不一定来自绘制函数，而更可能来自控件树初始化与重布局。

## 优先级建议

- `P0`: 启动页文件面板与模板页。
- `P1`: 启动阶段主窗口附属控件初始化。
- `P2`: startup tab 模式切换链。
