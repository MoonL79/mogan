# 模块 08: 网络与外部资源

## 关键热点列表

- `get_from_web()` 是同步 HEAD + 同步下载，且显式承认会阻塞主线程，见 `src/System/Files/web_files.cpp:113-146`。
- 模板中心远端元数据和下载基于 `QNetworkAccessManager` 异步处理，但下载完成后立即在主线程创建目录、读 reply、写文件，见 `src/Mogan/TemplateCenter/template_api.cpp:166-209`。
- `TemplateManager::initialize()` 总是调用 `refreshTemplates()`，即使已有缓存也会在启动后立刻触发远端刷新，见 `src/Mogan/TemplateCenter/template_manager.cpp:85-100`。
- Qt UI 层的登录、网络可用性检查、版本检查各自散落创建 `QNetworkAccessManager`，缺统一复用与统一超时策略，见 `src/Plugins/Qt/qt_tm_widget.cpp:2335-2425`、`src/Plugins/Qt/QTMOAuth.cpp:187-266`。

## 可验证假设

- 真正会造成“卡死感”的网络路径主要来自 `web_files.cpp`，不是模板中心那类异步请求。
- 模板下载完成后的文件写入和解析回调会在低端设备上造成次级卡顿。
- 线上网络抖动时，缺统一超时/退避会放大启动后后台噪音与 UI 线程回调密度。

## 优先级建议

- `P0`: `web_files.cpp` 同步网络路径。
- `P1`: 模板中心启动后立即刷新和下载落盘。
- `P2`: 零散网络管理器与超时策略统一性。
