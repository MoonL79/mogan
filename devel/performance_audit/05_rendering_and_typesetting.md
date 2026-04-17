# 模块 05: 渲染与排版

## 关键热点列表

- `typesetter_rep::typeset()` 在完整排版路径中直接 `br->typeset(...)` 后构建 `pager` 并 `make_pages()`，是偏全量的排版装配链，见 `src/Typeset/Bridge/typesetter.cpp:140-178`。
- 即使有变更矩形收敛，`typesetter_rep::typeset(SI&,...)` 仍需要重新生成 box 树、收集 change log、重新计算 page colors，见 `src/Typeset/Bridge/typesetter.cpp:181-223`。
- `lazy_document_rep::produce()` 会遍历所有段落，逐段 produce 后再 merge stack；其“lazy”更多是延后物化某些节点，不是天然首屏优先，见 `src/Typeset/Line/lazy_typeset.cpp:66-98`。
- 文档变化后 `tm_server_rep::typeset_update_all()` 直接让所有视图 `typeset_invalidate_all()`，多视图时重算范围可能迅速放大，见 `src/Texmacs/Server/tm_server.cpp:296-299`。

## 可验证假设

- 大文档首屏慢的主要来源仍是全量文档/页面排版，而不是单次绘制。
- 视图越多、分页色彩/页码引用越复杂，单次 typeset 成本越高。
- 当前 lazy 框架对“首屏优先”帮助有限，更像是分层组织而非真正按 viewport 裁剪。

## 优先级建议

- `P0`: `typesetter_rep::typeset` 全链路。
- `P1`: 多视图 `typeset_invalidate_all`。
- `P2`: lazy 结构的首屏收益评估。
