# 模块 04: 字体系统

## 关键热点列表

- `font_database_load()` 首次加载时会尝试本地缓存；若缓存空，则读取全局数据库、做过滤、扩展本地 truetype 字体目录，再写回本地缓存，同步成本很高，见 `src/Graphics/Fonts/font_database.cpp:297-323`。
- 字体数据库不仅加载 family/style，还顺带加载 feature、characteristics、substitution，首启成本覆盖面大，见 `src/Graphics/Fonts/font_database.cpp:198-223`、`277-323`。
- TeX 字体加载链在 `load_tex_tfm()` 中尝试多种尺寸回退、缓存命中、生成 tfm 文件、错误记录，失败路径很长，见 `src/Plugins/Metafont/load_tex.cpp:97-215`。
- `resolve_tex()` 仅对成功结果做 `font_basename.scm` 缓存；找不到字体时仍反复走 `resolve`/`kpsewhich`，见 `src/Plugins/Metafont/tex_files.cpp:93-117`。

## 可验证假设

- 首次字体冷启动慢主要来自 `font_database_load()` 的本地缓存生成和字体目录扩展，而不是单个字形 rasterize。
- 文档首次出现稀有 TeX 字体时，`load_tex_tfm()` 的尺寸回退和外部命令调用会形成明显长尾。
- 重启后仍慢的场景大概率来自字体缓存没有覆盖失败路径或字体目录校验使缓存失效。

## 优先级建议

- `P0`: `font_database_load` 的首启路径。
- `P1`: TeX 字体定位与回退链。
- `P2`: 错误字体记录与外部命令生成链。
