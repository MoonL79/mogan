# C++ 智能指针迁移报告

## 项目概述

将 Mogan STEM 项目中手写的引用计数指针系统替换为 C++11 标准库的 `std::shared_ptr`。

---

## ✅ 已完成迁移

### Kernel/Types 模块 (5 个文件)

**状态**: ✅ 完成并通过编译验证

#### 修改的文件:
1. **src/Kernel/Types/tab.hpp**
   - 移除 `concrete_struct` 继承
   - 使用 `std::shared_ptr<tab_rep>` 替代裸指针
   - 使用 `std::make_shared` 替代 `tm_new`
   - 添加标准运算符重载 (->, *, ==, !=)

2. **src/Kernel/Types/space.hpp/cpp**
   - 同上模式迁移
   - 更新构造函数使用初始化列表

3. **src/Kernel/Types/rectangles.hpp/cpp**
   - 同上模式迁移

#### 编译验证:
```bash
$ xmake b stem
[100%]: build ok, spent 22.633s
```

#### 迁移模式示例:

**之前** (手写引用计数):
```cpp
class tab_rep : concrete_struct {
  // ...
};

class tab {
  CONCRETE (tab);
  inline tab () : rep (tm_new<tab_rep> ()) {}
};
CONCRETE_CODE (tab);
```

**之后** (std::shared_ptr):
```cpp
#include <memory>

class tab_rep {
  // ...
};

class tab {
  std::shared_ptr<tab_rep> rep;
public:
  inline tab () : rep (std::make_shared<tab_rep> ()) {}
  
  inline tab_rep* operator->() { return rep.get(); }
  inline tab_rep& operator*() { return *rep; }
};
```

---

## 📋 待迁移模块

### Typeset 模块 (12 个文件)

**复杂度**: 高
**依赖关系**: 复杂

#### 文件列表:
1. `src/Typeset/boxes.hpp` - 核心类型，8 处 CONCRETE/ABSTRACT
2. `src/Typeset/Bridge/bridge.hpp` - 桥接类型
3. `src/Typeset/Table/table.hpp` - 表格类型
4. `src/Typeset/Page/vpenalty.hpp`
5. `src/Typeset/Page/skeleton.hpp`
6. `src/Typeset/env.hpp`
7. `src/Typeset/Boxes/construct.hpp`
8. `src/Typeset/Boxes/xkerning.hpp`
9. `src/Typeset/Format/page_item.hpp`
10. `src/Typeset/Format/line_item.hpp`
11. `src/Typeset/Format/stack_border.hpp`
12. `src/Typeset/Concat/canvas_properties.hpp`

#### 迁移难点:

1. **相互依赖**: 类型之间存在复杂的相互依赖关系
   - `box` 包含 `table`
   - `table` 包含 `cell`
   - `cell` 包含 `box` 和 `table`

2. **CONCRETE_NULL 宏**: 支持空值的智能指针需要特殊处理
   ```cpp
   class table {
     CONCRETE_NULL (table);
     // ...
   };
   CONCRETE_NULL_CODE (table);
   ```

3. **继承体系**: `concrete_struct` 和 `abstract_struct` 的使用

### Graphics/Mathematics 模块 (多个文件)

**复杂度**: 中

使用 `CONCRETE_TEMPLATE` 宏的模板类:
- `vector<T>`
- `polynomial<T>`
- `matrix<T>`

---

## 🎯 迁移策略建议

### 阶段 1: 简单独立类型 ✅ (已完成)
- Kernel/Types 模块

### 阶段 2: 核心基础设施
**建议顺序**:
1. `boxes.hpp` - 最核心类型
2. `bridge.hpp` - 桥接基础设施
3. 其他 Format 类型

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
| `CONCRETE_NULL(PTR)` | `std::shared_ptr<PTR##_rep> rep;` + 空指针处理 |
| `CONCRETE_NULL_CODE(PTR)` | is_nil 函数 + 默认构造函数 |
| `ABSTRACT(PTR)` | 类似 CONCRETE，支持多态 |
| `tm_new<T>(args)` | `std::make_shared<T>(args)` |
| `concrete_struct` | 移除（不需要基类）|
| `tm_delete(ptr)` | 不需要（自动管理）|

### 性能考虑

**std::shared_ptr 优缺点**:
- ✅ 标准 C++，易于理解和维护
- ✅ 线程安全引用计数
- ✅ 与标准库和第三方库兼容
- ⚠️ 轻微性能开销（原子操作）
- ⚠️ 内存占用略高（控制块）

**建议**:
- 对于频繁创建/销毁的小对象，考虑使用 `std::unique_ptr`
- 对于确定单一所有权的对象，使用 `std::unique_ptr`
- 对于需要共享所有权的对象，使用 `std::shared_ptr`

---

## 🚀 下一步行动建议

### 选项 1: 继续完整迁移 (推荐)
**工作量**: 2-3 天
**风险**: 中（依赖关系复杂）
**收益**: 完整的现代化代码库

执行步骤:
1. 迁移 Typeset/boxes.hpp (核心)
2. 迁移 Typeset/Bridge/bridge.hpp
3. 批量迁移剩余 Typeset 文件
4. 迁移 Graphics/Mathematics 模板类
5. 全面编译验证

### 选项 2: 保持现状
**理由**: 当前手写系统工作正常
**风险**: 低
**缺点**: 维护成本高，与现代 C++ 不兼容

---

## 📊 完成度统计

| 模块 | 文件数 | 状态 | 完成度 |
|------|--------|------|--------|
| Kernel/Types | 5 | ✅ 完成 | 100% |
| Typeset | 12 | ⏳ 待开始 | 0% |
| Graphics/Math | ~5 | ⏳ 待开始 | 0% |
| **总计** | **22+** | - | **~23%** |

---

## 💡 关键经验

1. **简单类型优先**: Kernel/Types 模块成功证明模式可行
2. **依赖顺序**: 需要先迁移被依赖的类型
3. **编译验证**: 每次修改后立即验证
4. **运算符重载**: 保持与原接口的兼容性

---

*报告生成时间*: 2026-04-17  
*分支*: modernization/shared_ptr  
*已验证*: Kernel/Types 编译通过 ✅
