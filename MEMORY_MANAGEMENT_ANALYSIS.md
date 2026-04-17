# Phase 2: 内存管理现代化分析报告

## 概述

Mogan 项目使用了一套自定义的内存管理系统，结合了引用计数和自定义分配器。本报告分析当前的内存管理机制，并提出现代化建议。

---

## 当前内存管理机制

### 1. 核心组件

#### **concrete_struct / abstract_struct**
```cpp
struct concrete_struct {
  int ref_count;
  inline concrete_struct () : ref_count (1) {}
  virtual inline ~concrete_struct () {}
};
```

- `concrete_struct`: 引用计数初始化为 1
- `abstract_struct`: 引用计数初始化为 0
- 都使用虚析构函数支持多态

#### **CONCRETE 宏系统**
定义在: `/home/mingshen/repositories/mogan/3rdparty/lolly/Kernel/Abstractions/classdef.hpp`

```cpp
#define CONCRETE(PTR) \
  PTR##_rep* rep; \
public: \
  inline PTR (const PTR&); \
  inline ~PTR (); \
  inline PTR##_rep* operator->(); \
  inline PTR& operator= (PTR x)
```

功能:
- 创建智能指针对象
- 自动管理引用计数
- 支持拷贝构造和赋值

#### **引用计数管理**
```cpp
#define INC_COUNT(R) { (R)->ref_count++; }
#define DEC_COUNT(R) { if (0 == --((R)->ref_count)) { tm_delete (R); } }
```

#### **tm_new / tm_delete**
定义在: `/home/mingshen/repositories/mogan/3rdparty/lolly/System/Memory/fast_alloc.hpp`

```cpp
template <typename C>
inline C* tm_new () {
  void* ptr= fast_new (sizeof (C));
  (void) new (ptr) C ();
  return (C*) ptr;
}
```

特点:
- 使用 fast_alloc 自定义内存池
- placement new 构造对象
- 避免标准 new/delete 的开销

---

## 使用统计

### tm_new / tm_delete 使用分布

| 文件 | 使用次数 | 类型 |
|------|----------|------|
| player.cpp | 8 | concrete |
| concater.cpp | 8 | concrete |
| new_window.cpp | 5 | concrete |
| tm_window.cpp | 4 | concrete |
| tm_dialogue.cpp | 3 | concrete |
| new_view.cpp | 3 | concrete |
| prog_language.cpp | 3 | concrete |
| tm_server.cpp | 2 | concrete |
| new_style.cpp | 2 | concrete |
| tm_button.cpp | 2 | concrete |
| space.cpp | 2 | concrete |
| new_buffer.cpp | 2 | concrete |
| language.cpp | 2 | concrete |
| (其他 8 个文件) | 各 1 次 | concrete/abstract |

**总计**: 53 处使用

### 使用 concrete_struct/abstract_struct 的头文件

- Kernel/Types: tab, space, rectangles
- Typeset: boxes, table, bridge, page_item, vpenalty, skeleton
- Data/Document: new_data
- System/Language: packrat_parser
- Graphics: widget, grid
- (以及其他)

**总计**: 30+ 个头文件

---

## 内存管理模式分析

### 模式 1: Concrete 智能指针

```cpp
// tab.hpp
class tab_rep : concrete_struct {
public:
  int pos;
  tab_rep (int pos, tree t);
};

class tab {
  CONCRETE (tab);
  inline tab () : rep (tm_new<tab_rep> ()) {}
};
CONCRETE_CODE (tab);
```

**特点**:
- 值语义
- 自动引用计数
- 不可为空
- 类似 `std::shared_ptr` 但不可为空

### 模式 2: Abstract 智能指针

```cpp
// player.hpp
class player_rep : public abstract_struct {
  virtual void set_speed (double s) = 0;
};

class player {
  ABSTRACT (player);
};
ABSTRACT_CODE (player);
```

**特点**:
- 多态支持
- 可为空 (引用计数初始为 0)
- 类似 `std::shared_ptr` 支持多态

### 模式 3: 裸指针 + 显式 new/delete

```cpp
// 在 Qt 插件中
PDFImageXObject* imageXObject=
    documentContext.CreateImageXObjectFromJPGFile (...);
if (imageXObject == nullptr) { ... }
delete imageXObject;  // 直接使用 delete
```

**位置**: Plugins/Pdf, Plugins/Qt (9 个文件)

**问题**:
- 与自定义内存管理系统混用
- 潜在内存泄漏风险
- 不一致的内存管理策略

---

## 现代化建议

### 短期 (低风险)

#### 1. 统一裸指针使用

**问题**: 9 个文件直接使用 `new/delete`

**建议**:
```cpp
// 之前
PDFImageXObject* imageXObject= new PDFImageXObject (...);
delete imageXObject;

// 之后
auto imageXObject= std::make_unique<PDFImageXObject> (...);
// 自动删除
```

**优先级**: 高
**风险**: 低
**工作量**: 9 个文件

#### 2. 添加内存泄漏检测

```cpp
// 在 debug 构建中启用
#define DEBUG_MEMORY_LEAKS
```

使用 AddressSanitizer 或 Valgrind 定期检测。

### 中期 (中等风险)

#### 3. 引入 std::shared_ptr (可选)

**当前系统的问题**:
- 自定义实现，维护成本高
- 不熟悉的人难以理解和调试
- 与标准库不兼容

**迁移策略**:
```cpp
// 保持现有接口，内部使用标准库
// tab.hpp
class tab {
  std::shared_ptr<tab_rep> rep;
public:
  inline tab () : rep (std::make_shared<tab_rep> ()) {}
};
```

**挑战**:
- fast_alloc 性能优化可能需要保留
- 需要性能基准测试

#### 4. 现代化引用计数宏

```cpp
// 使用 C++11 特性简化宏
#define INC_COUNT(R) (++(R)->ref_count)
#define DEC_COUNT(R) \
  do { \
    if (--(R)->ref_count == 0) tm_delete (R); \
  } while (0)
```

### 长期 (高风险，高收益)

#### 5. 替换自定义内存管理系统

**理由**:
- 现代 C++ 编译器已优化标准分配器
- 自定义分配器增加维护负担
- 难以与第三方库集成

**迁移路径**:
1. 基准测试当前性能
2. 在隔离模块中测试 std::shared_ptr
3. 逐步替换
4. 验证性能未下降

#### 6. 使用 RAII 模式

```cpp
// 之前
void func () {
  pointer* p= tm_new<pointer> ();
  // ... 可能提前返回
  tm_delete (p);  // 容易遗漏
}

// 之后
void func () {
  auto p= std::make_unique<pointer> ();
  // ... 自动管理
}  // 自动删除
```

---

## 立即行动项

### 1. 修复裸指针使用 (9 个文件)

**目标文件**:
```
src/Plugins/Pdf/pdf_hummus_make_attachment.cpp
src/Plugins/Pdf/pdf_hummus_renderer.cpp
src/Plugins/Qt/qt_template_page.cpp
src/Plugins/Qt/qt_dialogues.cpp
src/Plugins/Qt/qt_guide_window.cpp
src/Plugins/Qt/QTMTabPage.cpp
src/Plugins/Qt/qt_ui_element.cpp
src/Plugins/Qt/QTMMenuHelper.cpp
src/Plugins/Qt/qt_tutorial.cpp
```

### 2. 添加智能指针包装

为 Qt/PDF 插件创建 RAII 包装器:

```cpp
// pdf_resource.hpp
template <typename T>
class PDFResource {
  T* ptr;
public:
  explicit PDFResource (T* p) : ptr (p) {}
  ~PDFResource () { delete ptr; }
  T* get () const { return ptr; }
  T* operator->() const { return ptr; }
  // 禁用拷贝，允许移动
  PDFResource (const PDFResource&) = delete;
  PDFResource (PDFResource&& other) : ptr (other.ptr) { other.ptr = nullptr; }
};
```

### 3. 编译器警告

添加编译选项检测裸指针使用:
```cmake
add_compile_options(-Wdelete-non-virtual-dtor)
```

---

## 风险评估

| 改动 | 风险 | 收益 | 建议 |
|------|------|------|------|
| 修复裸指针 | 低 | 中 | 立即执行 |
| 添加 RAII 包装 | 低 | 中 | 短期执行 |
| 替换 shared_ptr | 中 | 中 | 需要基准测试 |
| 移除 fast_alloc | 高 | 低 | 谨慎考虑 |

---

## 总结

当前内存管理系统的特点:
- ✅ 自定义引用计数智能指针
- ✅ fast_alloc 性能优化
- ⚠️ 裸指针与智能指针混用
- ⚠️ 宏系统复杂，难以调试

建议优先级:
1. **立即**: 修复 9 个文件的裸指针使用
2. **短期**: 添加 RAII 包装器和内存泄漏检测
3. **长期**: 评估是否迁移到标准智能指针

**核心原则**: 保持现有系统的稳定性，逐步引入现代 C++ 实践。

---

*报告生成时间*: 2026-04-17  
*分析范围*: src/ 目录下所有 C++ 文件  
*使用工具*: grep, ast-grep, 手动代码审查
