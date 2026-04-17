# C++ 智能指针迁移报告

## 项目概述

将 Mogan STEM 项目中手写的引用计数指针系统替换为 C++11 标准库的 `std::shared_ptr`。

---

## ✅ 已完成迁移

### Kernel/Types 模块 (5 个文件) ✅

**状态**: 完成并通过编译验证

#### 修改的文件:
1. **src/Kernel/Types/tab.hpp**
2. **src/Kernel/Types/space.hpp/cpp**
3. **src/Kernel/Types/rectangles.hpp/cpp**

#### 关键改进:
- 移除 `concrete_struct` 继承
- 使用 `std::shared_ptr` 替代裸指针
- 使用 `std::make_shared` 替代 `tm_new`

---

### Typeset/Format & Typeset/Page 模块 (3 个文件) ✅

**状态**: 完成并通过编译验证

#### 修改的文件:
1. **src/Typeset/Format/stack_border.hpp**
   - 简单 CONCRETE 类型迁移

2. **src/Typeset/Page/vpenalty.hpp**
   - 包含自定义运算符的迁移
   - operator==, operator!=, operator<, operator+

3. **src/Typeset/Page/skeleton.hpp**
   - 复杂类型：insertion 和 pagelet
   - CONCRETE_NULL 类型支持空值
   - 添加 is_nil() 函数
   - operator<< 成员函数

#### 编译验证:
```bash
$ xmake b stem
[100%]: build ok, spent 32.058s
```

---

## 📊 迁移完成度

| 模块 | 文件数 | 状态 | 完成度 |
|------|--------|------|--------|
| **Kernel/Types** | 5 | ✅ 完成 | 100% |
| **Typeset/Format** | 1 | ✅ 完成 | 100% |
| **Typeset/Page** | 2 | ✅ 完成 | 100% |
| Typeset/Table | 1 | ⏳ 待开始 | 0% |
| Typeset/Boxes | 1 | ⏳ 待开始 | 0% |
| Typeset/Bridge | 1 | ⏳ 待开始 | 0% |
| Typeset/其他 | 5 | ⏳ 待开始 | 0% |
| Graphics/Math | ~5 | ⏳ 待开始 | 0% |
| **总计** | **22+** | - | **~36%** |

---

## 📋 待迁移模块

### Typeset 核心模块 (6 个文件)

**复杂度**: 高
**依赖关系**: 复杂

#### 文件列表:
1. `src/Typeset/boxes.hpp` - 核心类型，8 处 CONCRETE/ABSTRACT，ABSTRACT_NULL(box)
2. `src/Typeset/Bridge/bridge.hpp` - 桥接类型
3. `src/Typeset/Table/table.hpp` - 表格类型，相互依赖复杂
4. `src/Typeset/Format/page_item.hpp` - 依赖 boxes
5. `src/Typeset/Format/line_item.hpp` - 依赖 boxes
6. `src/Typeset/env.hpp`, `Boxes/construct.hpp`, 等

#### 迁移难点:

1. **相互依赖**: 类型之间存在复杂的相互依赖关系
   - `box` 包含 `table`
   - `table` 包含 `cell`
   - `cell` 包含 `box` 和 `table`

2. **ABSTRACT_NULL 宏**: box 使用 ABSTRACT_NULL，需要特殊处理
   ```cpp
   class box {
     ABSTRACT_NULL (box);
     // ...
   };
   ```

3. **CONCRETE_NULL 宏**: table 和 cell 使用 CONCRETE_NULL

### Graphics/Mathematics 模块 (多个文件)

**复杂度**: 中

使用 `CONCRETE_TEMPLATE` 宏的模板类:
- `vector<T>`
- `polynomial<T>`
- `matrix<T>`

---

## 🎯 迁移策略建议

### 阶段 1: 简单独立类型 ✅ (已完成)
- Kernel/Types 模块 ✅
- Typeset/Format ✅
- Typeset/Page ✅

### 阶段 2: 核心基础设施 (下一步)
**建议顺序**:
1. `boxes.hpp` - 最核心类型，但复杂度高
2. `bridge.hpp` - 桥接基础设施
3. `table.hpp` - 需要与 boxes 同时考虑

### 阶段 3: 复杂相互依赖类型
- Table/Cell 系统
- 需要同时迁移以确保编译通过

### 阶段 4: 模板类型
- vector<T>
- polynomial<T>
- matrix<T>

---

## 📝 技术细节

### 宏替换对照表

| 旧宏 | 新实现 |
|------|--------|
| `CONCRETE(PTR)` | `std::shared_ptr<PTR##_rep> rep;` + 构造函数 |
| `CONCRETE_CODE(PTR)` | 内联运算符定义 |
| `CONCRETE_NULL(PTR)` | `std::shared_ptr<PTR##_rep> rep;` + is_nil() |
| `CONCRETE_NULL_CODE(PTR)` | is_nil 友元函数 |
| `ABSTRACT(PTR)` | 类似 CONCRETE，支持多态 |
| `ABSTRACT_NULL(PTR)` | CONCRETE_NULL + 继承支持 |
| `tm_new<T>(args)` | `std::make_shared<T>(args)` |
| `concrete_struct` | 移除（不需要基类）|
| `tm_delete(ptr)` | 不需要（自动管理）|

### 模式示例

**CONCRETE_NULL 迁移** (以 pagelet 为例):

```cpp
// 之前
struct pagelet {
  CONCRETE_NULL (pagelet);
  // ...
};
CONCRETE_NULL_CODE (pagelet);

// 之后
struct pagelet {
  std::shared_ptr<pagelet_rep> rep;
  
  inline pagelet () = default;
  inline pagelet (space ht);
  inline bool is_nil () const { return rep == nullptr; }
  
  inline pagelet_rep* operator->() { return rep.get(); }
  inline pagelet_rep& operator*() { return *rep; }
  
  friend bool is_nil (pagelet pg) { return pg.is_nil(); }
};
```

---

## 🚀 下一步行动建议

### 继续迁移 (推荐)
**剩余工作量**: 
- Typeset 核心模块: 6 个文件
- Graphics/Math: ~5 个文件

**挑战**:
- boxes.hpp 是最核心的类型，影响面广
- 需要仔细处理 ABSTRACT_NULL 宏
- 建议同时迁移相互依赖的类型

### 保持现状
- 已完成约 36% 的迁移
- 核心基础设施（Kernel/Types, Format, Page）已现代化
- 剩余模块保持手写引用计数也可以正常工作

---

## 💡 关键经验

1. **简单类型优先**: Kernel/Types 和 Format/Page 成功证明模式可行
2. **CONCRETE_NULL 处理**: 需要手动添加 is_nil() 函数
3. **运算符重载**: 保持与原接口的兼容性
4. **编译验证**: 每次修改后立即验证

---

## 📁 分支和提交

**分支**: `modernization/shared_ptr`

**提交历史**:
```
73731d3c2 modernization: migrate Typeset/Format and Typeset/Page to std::shared_ptr
4095ed98d modernization: migrate Kernel/Types to std::shared_ptr
```

---

*报告更新*: 2026-04-17  
*已完成*: Kernel/Types (100%), Typeset/Format (100%), Typeset/Page (100%)  
*待完成*: Typeset 核心模块, Graphics/Math  
*编译状态*: ✅ 100% 通过
