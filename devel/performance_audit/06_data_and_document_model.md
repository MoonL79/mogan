# 模块 06: 数据模型与文档模型

## 关键热点列表

- `buffer_import()` 同步 `import_tree()`，后者先 `resolve` 再完整读取字符串并做 `generic_to_tree` 转换，导入与 UI 线程耦合明显，见 `src/Texmacs/Data/new_buffer.cpp:497-529`。
- `import_loaded_tree()` 在格式推断、链接注册、子格式附着之间多次构造/复制 tree，数据构建偏整块，见 `src/Texmacs/Data/new_buffer.cpp:481-495`。
- `load_style_tree()` 把样式文件完整 `load_string` 后转成文档树，并以包名做进程内缓存；首次访问每个 style 仍是全量载入，见 `src/Texmacs/Data/new_buffer.cpp:532-550`。
- 文档变更通知会级联到编辑器和 typesetter，环境变化尤其容易推动整块刷新，见 `src/Texmacs/Data/new_buffer.cpp:366-374`。

## 可验证假设

- 大文件打开慢不仅来自 IO，还来自一次性 parse/import/tree 构建。
- 样式和代码子格式首次命中时会出现明显额外开销。
- 文档树与 UI/typeset 的通知耦合会让局部修改触发过宽的后续工作。

## 优先级建议

- `P0`: 文档导入与格式转换链。
- `P1`: style tree 首次加载。
- `P2`: 环境变化后的通知传播宽度。
