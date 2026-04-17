# 模块 11: 基础设施与历史抽象

## 关键热点列表

- `concrete_struct` 与 `CONCRETE` 宏体系使用手写引用计数和宏展开的所有权语义，调试和性能归因成本都较高，见 `3rdparty/lolly/Kernel/Abstractions/classdef.hpp:24-38`、`139-170`。
- `tm_new/tm_delete` 建立在自定义 `fast_alloc` 之上，几乎贯穿核心对象创建路径；性能优化空间存在，但可观测性和与现代 profiler 的语义映射较弱，见 `3rdparty/lolly/System/Memory/fast_alloc.hpp:27-35`、`57-239`。
- 自定义 `hashmap/list/string` 都是引用计数实现，热点代码大量按值传递这些对象；虽然共享底层 rep，但读写路径和 copy-on-write 语义对性能直觉不透明，见 `3rdparty/lolly/Kernel/Containers/hashmap.hpp:47-76`、`176-213` 与 `3rdparty/lolly/Kernel/Types/string.hpp:24-69`。
- `hashmap` 直接向 `edit_env_rep` 暴露 friend，说明历史抽象边界已经被打穿，后续性能治理会受限于模块内聚度，见 `3rdparty/lolly/Kernel/Containers/hashmap.hpp:160-165`。

## 可验证假设

- 底层抽象不会是当前第一批体感性能问题的主因，但会明显限制长期性能治理和工具化。
- 若做全局 profiling，分配热点会高度集中在 `tm_new`/自定义容器上，但短期很难直接转化为用户体感收益。
- 所有权与共享语义不透明，会放大“看不见的拷贝/写放大”排查成本。

## 优先级建议

- `P2`: 作为长期治理层保留，不建议在前几轮体感性能优化中先动大刀。
- `前置条件`: 先完成启动、主线程、文件扫描、字体、排版的可量化治理，再回头处理基础设施。
