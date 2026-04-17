# 模块 03: 资源定位与文件系统扫描

## 关键热点列表

- `plugin_path()` 基于 `$TEXMACS_HOME_PATH:$TEXMACS_PATH` 拼出 wildcard 搜索，再走 `complete/expand`，启动期多次用于 `progs/bin/lib/styles/packages/doc` 路径构造，目录数一多就会放大成本，见 `src/System/Boot/init_texmacs.cpp:282-287`、`404-476`。
- `init_env_vars()` 启动时调用 `search_sub_dirs` 递归展开 style/text/package 路径，属于高扇出目录扫描，见 `src/System/Boot/init_texmacs.cpp:431-447`。
- TeX 字体定位链先 `resolve(the_*_path * name)`，失配时再调用外部 `kpsewhich`；当前只有正缓存，没有系统级负缓存，缺字体时会重复查找，见 `src/Plugins/Metafont/tex_files.cpp:50-81`、`93-117`。
- `data_cache::is_recursively_up_to_date()` 在初始化时递归遍历字体目录树做目录有效性检查，见 `src/System/Misc/data_cache.cpp:87-98`、`213-233`。

## 可验证假设

- 启动期 `exists/resolve/search_sub_dirs/read_directory` 调用次数会非常高，尤其集中在样式、插件和字体目录。
- 首次慢、二次快主要来自目录枚举和路径缓存命中，而不是纯 CPU 初始化。
- 缺失字体或缺失资源时，性能会因负缓存缺失而显著恶化。

## 优先级建议

- `P0`: `init_env_vars` 与 `plugin_path/search_sub_dirs` 链路。
- `P1`: TeX 字体路径解析及 `kpsewhich` 回退。
- `P2`: 目录有效性递归检查。
