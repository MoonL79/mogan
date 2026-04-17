# 模块 10: 后台任务与任务模型

## 关键热点列表

- `BootstrapTaskExecutor` 是单线程初始化执行器，注释与实现都确认它只是把 `init_texmacs` 拆成几段主线程任务，不是真异步，见 `src/Plugins/Qt/qt_guide_task_executor.cpp:1-18`、`195-239`。
- 多处 `QTimer::singleShot(0/10ms)` 被用作“延迟执行”，例如启动初始化、模板中心初始化、文件页卡片重排、更新检查；这改善了事件循环时序，但不改变执行线程，见 `src/Plugins/Qt/qt_guide_task_executor.cpp:104`、`390-397`、`src/Plugins/Qt/qt_template_page.cpp:127-128`、`src/Plugins/Qt/qt_file_page.cpp:233`。
- 缩略图和 PDF 预览缓存使用 `QMetaObject::invokeMethod` 回主线程更新 UI，后台计算与主线程提交边界相对清晰，但仍需关注提交粒度，见 `src/Mogan/Cache/thumbnail_cache.cpp:88` 与 `src/Mogan/Cache/pdf_preview_cache.cpp:87`。
- 模板缓存写盘、类别缓存锁文件、模板下载落盘都在完成回调内直接做，没有统一任务队列或后端执行器，见 `src/Mogan/TemplateCenter/template_cache.cpp:337-381`、`435-474`、`src/Mogan/TemplateCenter/template_api.cpp:183-207`。

## 可验证假设

- 当前系统大量“异步”只是延后到下一拍执行，主线程长任务总量并未实质减少。
- 真后台化做得较好的区域主要是图片/预览缓存；启动、模板、字体仍然偏串行。
- 如果统计主线程任务长度，延迟调度后的任务块仍会呈现明显长尾。

## 优先级建议

- `P0`: 启动任务模型的真异步/假异步边界。
- `P1`: 模板与缓存写盘任务的执行模型。
- `P2`: UI 更新回主线程的批量粒度。
