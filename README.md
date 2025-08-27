# Mini_Engine

**Mini_Engine** 是一个基于 **C++ / Vulkan** 开发的轻量级游戏引擎，旨在探索现代游戏引擎的架构与实现方法。  
项目从底层渲染到上层编辑器，构建了一个完整的实验性引擎框架，适合学习和扩展。

---

## ✨ 特性
- **渲染管线**：支持基础的 Vulkan 渲染流程，包含多光源、阴影、后处理等现代图形特性  
- **资源管理**：实现对纹理、网格、动画等资产的统一加载与序列化管理  
- **编辑器支持**：内置轻量级 UI 编辑器，支持场景可视化与交互式调试  
- **模块化设计**：核心功能模块化，方便扩展物理、粒子、特效等系统  
- **工具链集成**：基于 **Git / CMake / Clang** 的跨平台开发流程，便于构建与调试  

---

## 🛠️ 架构
项目分为三个核心部分：

- **Runtime**：引擎运行时，包含渲染、资源、物理、粒子等核心系统  
- **Editor**：图形化编辑器，支持场景管理、资源可视化与交互工具  
- **Parser / Reflection**：元数据与反射系统，支撑序列化和资源编辑  

---

## 📦 构建
```bash
git clone https://github.com/yourname/Mini_Engine.git
cd Mini_Engine
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug

---

## 🚀 运行
```bash
./build/engine/source/editor/MomoEditor
