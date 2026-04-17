# Mogan Performance Audit

- [00_todolist.md](./00_todolist.md)
- [01_startup_performance.md](./01_startup_performance.md)
- [02_main_thread_blocking.md](./02_main_thread_blocking.md)
- [03_resource_location_and_fs_scan.md](./03_resource_location_and_fs_scan.md)
- [04_font_system.md](./04_font_system.md)
- [05_rendering_and_typesetting.md](./05_rendering_and_typesetting.md)
- [06_data_and_document_model.md](./06_data_and_document_model.md)
- [07_cache_system.md](./07_cache_system.md)
- [08_network_and_external_resources.md](./08_network_and_external_resources.md)
- [09_qt_ui_layer.md](./09_qt_ui_layer.md)
- [10_background_tasks_and_task_model.md](./10_background_tasks_and_task_model.md)
- [11_infrastructure_and_legacy_abstractions.md](./11_infrastructure_and_legacy_abstractions.md)

说明:

- 本轮报告以静态代码排查为主，按用户指定顺序完成。
- 每个模块只保留三项输出: 关键热点列表、可验证假设、优先级建议。
- 行号引用基于当前工作区代码快照。
