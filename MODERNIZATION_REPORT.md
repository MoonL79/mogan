# Mogan STEM - C++ 现代化重构报告

## 项目信息
- **仓库**: ~/repositories/mogan
- **分支**: modernization/nullptr
- **C++ 标准**: C++17
- **代码规模**: 672+ C++ 文件

---

## 已完成的现代化工作

### Phase 1.1: NULL → nullptr 替换 ✅

**状态**: 已完成并提交

**变更统计**:
- 修改文件数: 89 个
- 插入/删除: 770 行更改
- 替换范围: src/ 目录下所有 .cpp 和 .hpp 文件

**涉及的模块**:
- System/Link (dyn_link, socket, server, client)
- Texmacs/Data (new_view, new_window, new_buffer, new_project)
- Texmacs/Server & Window
- Edit/Interface (edit_interface, edit_cursor, edit_mouse)
- Edit/Editor (edit_typeset, edit_main)
- Edit/Process
- Typeset/Table & Boxes
- Plugins/Qt
- Scheme/L5 & S7
- Mogan/Research
- 以及更多...

**提交信息**:
```
modernization: replace NULL with nullptr

Replace all C-style NULL macros with C++11 nullptr keyword.
This is a mechanical change that improves type safety and
makes the codebase more modern.

- 89 files changed
- 770 insertions/deletions
- No functional changes
```

---

### Phase 1.2: C风格强制类型转换 → C++ 风格转换 (部分完成)

**状态**: 部分完成

**已完成文件**:
- src/Plugins/Pdf/pdf_image.cpp - 8处转换替换
- src/Plugins/Qt/QTMOAuth.cpp - 8处转换替换

**示例改进**:
```cpp
// 之前
(char*) f
(unsigned char*) (char*) buf
(void*) imageXObject == nullptr

// 之后
static_cast<char*> (f)
reinterpret_cast<unsigned char*> (static_cast<char*> (buf))
imageXObject == nullptr  // 移除了不必要的(void*)转换
```

**提交信息**:
```
modernization: replace C-style casts with C++ casts in QTMOAuth.cpp

Replace C-style (char*) casts with static_cast<char*>() for better
type safety and modern C++ compliance.
```

---

## 已识别的后续现代化机会

### Phase 2: 内存管理现代化 (待进行)

**发现的问题**:
1. **自定义内存管理**: 使用 `tm_new`/`tm_delete`, `concrete_struct`, `abstract_struct`
2. **原始指针**: 大量裸指针可替换为智能指针 (std::unique_ptr, std::shared_ptr)
3. **手动 new/delete**: 16+ 处显式内存管理

**建议改进**:
- 将 `tm_new<T>()` 替换为 `std::make_unique<T>()`
- 使用 RAII 模式管理资源
- 考虑逐步替换自定义内存管理机制

**涉及文件**:
- src/Plugins/Qt/qt_template_page.cpp (7处 new)
- src/Plugins/Qt/qt_guide_window.cpp (2处 new)
- src/Plugins/Pdf/pdf_hummus_*.cpp
- 以及其他 Qt 插件文件

---

### Phase 3: 现代 C++ 特性应用 (待进行)

**潜在改进**:

1. **auto 关键字**: 减少冗长类型声明
   ```cpp
   // 之前
   std::map<std::string, std::vector<int>>::iterator it = map.begin();
   
   // 之后
   auto it = map.begin();
   ```

2. **范围 for 循环**: 简化容器遍历
   ```cpp
   // 之前
   for (int i = 0; i < vec.size(); ++i) { ... }
   
   // 之后
   for (const auto& elem : vec) { ... }
   ```

3. **constexpr**: 替代宏定义
   ```cpp
   // 之前
   #define MAX_SIZE 100
   
   // 之后
   constexpr int MAX_SIZE = 100;
   ```

4. **using 替代 typedef**:
   ```cpp
   // 之前
   typedef std::vector<int> IntVec;
   
   // 之后
   using IntVec = std::vector<int>;
   ```

5. **std::string_view**: 优化字符串参数传递
   ```cpp
   // 之前
   void func(const std::string& s);
   
   // 之后
   void func(std::string_view s);
   ```

6. **std::optional**: 处理可选值
   ```cpp
   std::optional<Result> compute();
   ```

---

## 代码质量改进

### 已配置的工具
- ✅ clang-format-19 (代码格式化)
- ✅ CI/CD 格式检查
- ✅ Xmake 构建系统 (C++17)

### 建议添加
- clang-tidy (静态分析)
- 现代 C++ 检查规则:
  - `cppcoreguidelines-*`
  - `modernize-*`
  - `performance-*`
  - `readability-*`

---

## 目录结构现代化映射

```
src/
├── Data/          (~52 文件)  数据模型与文档处理
├── Edit/          (~81 文件)  编辑器核心 ⭐重点
├── Graphics/      (~47 文件)  图形渲染
├── Kernel/        (~8 文件)   核心类型 ⭐优先
├── Mogan/         (~16 文件)  应用核心
├── Plugins/       (~190 文件) 插件系统 ⭐重点
├── Scheme/        (~16 文件)  语言绑定
├── System/        (~46 文件)  系统底层
├── Texmacs/       (~25 文件)  兼容层
└── Typeset/       (~103 文件) 排版引擎
```

---

## 建议的后续步骤

### 短期 (1-2 周)
1. 完成剩余的 C 风格强制类型转换替换
2. 添加 clang-tidy 配置文件
3. 运行静态分析并修复明显问题

### 中期 (1-2 月)
1. 核心模块智能指针化 (Kernel, Data)
2. 引入 auto 和范围 for 循环
3. 添加单元测试框架 (Google Test)

### 长期 (3-6 月)
1. 重构自定义内存管理系统
2. 模块化核心组件
3. 性能优化与基准测试

---

## 验证策略

### 编译验证
```bash
cd ~/repositories/mogan
xmake config --mode=releasedbg
xmake build
```

### 代码格式验证
```bash
clang-format-19 --dry-run --Werror src/**/*.cpp
```

### 回归测试
- 确保所有现有功能正常工作
- 检查跨平台兼容性 (Linux, macOS, Windows)

---

## 总结

本次现代化重构完成了第一阶段的基础改进:
- ✅ **NULL → nullptr**: 89 文件, 770 行更改
- ✅ **C风格转换**: 部分关键文件已改进
- 📋 **后续计划**: 内存管理、现代特性、静态分析

**立即收益**:
- 类型安全性提升
- 代码更现代、更易维护
- 为后续 C++17/20 特性应用奠定基础

**风险**: 低 (纯语法改进, 无功能变化)

---

*报告生成时间*: 2026-04-17  
*分支*: modernization/nullptr  
*提交数*: 2
