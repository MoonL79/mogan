# 模块 02: 主线程阻塞

## 关键热点列表

- `get_from_web` 在 Qt 模式下显式 `processEvents` 后继续同步 `http_head` 和同步下载，这不是异步 UI，而是典型主线程阻塞网络 IO，见 `src/System/Files/web_files.cpp:111-146`。
- 启动页模板中心初始化通过 `QTimer::singleShot(0)` 触发，但 `TemplateManager::initialize()` 内部直接做本地缓存读取、Scheme 解析、远端刷新触发，仍在 UI 线程执行，见 `src/Plugins/Qt/qt_template_page.cpp:125-129` 与 `src/Mogan/TemplateCenter/template_manager.cpp:67-100`。
- 启动页文件面板在 `showEvent` 中刷新最近文档、启动定时器并重排卡片；`rearrangeStyleCards()` 每次都先 remove 再 add 全量卡片，窗口宽度变化时会反复触发布局，见 `src/Plugins/Qt/qt_file_page.cpp:228-248`、`273-295`。
- `checkNetworkAvailable()`、`checkVersionUpdate()` 每次各自新建 `QNetworkAccessManager`，虽然请求异步，但对象生命周期与 UI 控件强耦合，响应处理仍回到主线程并夹带 Scheme 调用，见 `src/Plugins/Qt/qt_tm_widget.cpp:2335-2425`。

## 可验证假设

- UI 卡顿峰值将与同步 web 下载、模板页首次初始化、文件页重排同时出现。
- 在主线程采样中，`QTimer::singleShot` 触发的初始化段仍会落在长帧区间，而不是后台线程。
- 模板中心和启动页文件面板在弱机上会造成首次显示时明显掉帧。

## 优先级建议

- `P0`: `web_files.cpp` 的同步网络路径。
- `P1`: 模板中心初始化与启动页文件面板重排。
- `P2`: 版本检查和网络探测等次要主线程回调。
