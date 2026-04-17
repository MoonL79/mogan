# 模块 07: 缓存体系

## 关键热点列表

- `data_cache` 主要是进程内 hashmap 加少量磁盘镜像；`cache_refresh()` 会整体重建三类缓存，没有更细粒度失效，见 `src/System/Misc/data_cache.cpp:146-210`。
- `is_up_to_date()` 和 `declare_out_of_date()` 明确留下 FIXME: 目录失效后不会精确清除相关条目，这意味着缓存一致性与性能都不理想，见 `src/System/Misc/data_cache.cpp:75-84`、`101-108`。
- `font_cache.scm`、`font_basename.scm` 为字体路径与尺寸回退提供磁盘缓存，但失败路径和目录扫描结果缓存不足，见 `src/System/Misc/data_cache.cpp:166-190`。
- 模板缓存系统会在初始化时读取 metadata/index/categories 三套 JSON，并在注册/清理时同步写回；缓存可持久化，但写盘仍在前台路径，见 `src/Mogan/TemplateCenter/template_cache.cpp:31-45`、`117-163`、`395-474`。

## 可验证假设

- “第一次慢、进程内第二次快、重启后部分回退”会是当前缓存体系的典型表现。
- 字体和资源搜索的缺失负缓存是长尾 miss 的主要来源。
- 模板缓存对网络结果有效，但对 UI 首次初始化并没有完全脱敏，因为读取和解析仍在主线程。

## 优先级建议

- `P0`: 字体/路径类缓存的失效与负缓存缺口。
- `P1`: `cache_refresh` 的粗粒度刷新。
- `P2`: 模板缓存的前台读写组织方式。
