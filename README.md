# CSAPP Labs

## Overview | 概述

This repository contains my solutions and implementation code for the labs from *Computer Systems: A Programmer's Perspective (CS:APP)*, 3rd Edition. These hands-on projects help deepen the understanding of fundamental computer systems concepts through practical implementation.

本仓库包含《深入理解计算机系统（CS:APP）》第3版中所有实验的解决方案和实现代码。这些动手实践项目通过实际实现帮助加深对计算机系统基础概念的理解。

---

## 🏗️ Project Structure | 项目结构

```
csapplabs/
├── datalab/          # 数据实验 - 位操作谜题
├── bomblab/          # 炸弹实验 - 逆向工程
├── attacklab/        # 攻击实验 - 漏洞利用
├── cachelab/         # 缓存实验 - 缓存模拟器
├── shlab/            # Shell实验 - Unix Shell实现
├── malloclab/        # 内存分配实验 - 动态内存分配器
└── proxylab/         # 代理实验 - 并发HTTP代理
```

---

## 📚 Lab List | 实验列表

### 1. 🎯 Data Lab | 数据实验
- **Description**: Solve a series of puzzles involving bit-level manipulations under strict coding constraints
- **描述**: 在严格的编码约束下解决一系列涉及位级操作的谜题
- **Topics**: Two's complement, floating-point representation, bitwise operations
- **主题**: 二进制补码、浮点数表示、位运算
- **Key Skills**: Bit manipulation, integer and floating-point representations
- **关键技能**: 位操作、整数和浮点数表示
- **Status**: ✅ Completed | 已完成


### 2. 💣 Bomb Lab | 炸弹实验
- **Description**: Reverse-engineer a "binary bomb" executable using GDB debugging to defuse six phases
- **描述**: 使用GDB调试反编译"二进制炸弹"可执行文件，拆除六个阶段的炸弹
- **Topics**: Assembly code analysis, debugging techniques, memory layout
- **主题**: 汇编代码分析、调试技术、内存布局
- **Key Skills**: GDB usage, assembly understanding, reverse engineering
- **关键技能**: GDB使用、汇编理解、逆向工程
- **Status**: ✅ Completed | 已完成


### 3. ⚔️ Attack Lab | 攻击实验
- **Description**: Exploit buffer overflow vulnerabilities through code injection and Return-Oriented Programming (ROP) attacks across five phases
- **描述**: 通过代码注入和面向返回编程（ROP）攻击利用缓冲区溢出漏洞，共五个阶段
- **Topics**: Stack smashing, code injection, ROP techniques
- **主题**: 栈溢出、代码注入、ROP技术
- **Key Skills**: Vulnerability exploitation, stack manipulation
- **关键技能**: 漏洞利用、栈操作
- **Status**: ✅ Completed | 已完成
- **Techniques**: Code injection (Phase 1-3), ROP (Phase 4-5) | 代码注入(1-3阶段), ROP(4-5阶段)

### 4. 💾 Cache Lab | 缓存实验
- **Description**: Implement a configurable cache simulator in C with LRU replacement policy
- **描述**: 用C语言实现可配置的缓存模拟器，采用LRU替换策略
- **Topics**: Cache memory hierarchy, replacement policies, simulation
- **主题**: 缓存内存层次结构、替换策略、模拟
- **Key Skills**: Cache design, performance optimization
- **关键技能**: 缓存设计、性能优化
- **Status**: ✅ Completed | 已完成
- **Features**: Configurable S/E/B parameters, LRU replacement | 可配置S/E/B参数，LRU替换策略

### 5. 🐚 Shell Lab (shlab) | Shell实验
- **Description**: Build a Unix shell supporting job control, signal handling, and built-in commands
- **描述**: 构建支持作业控制、信号处理和内置命令的Unix shell
- **Topics**: Process control, signal handling, job management
- **主题**: 进程控制、信号处理、作业管理
- **Key Skills**: Process creation, signal handling, shell implementation
- **关键技能**: 进程创建、信号处理、shell实现
- **Status**: ✅ Completed | 已完成
- **Features**: 
  - Job control with `&`, `fg`, `bg` | 支持`&`, `fg`, `bg`的作业控制
  - Signal handling for Ctrl-C, Ctrl-Z | Ctrl-C, Ctrl-Z信号处理
  - Built-in commands: `jobs`, `quit` | 内置命令: `jobs`, `quit`

### 6. 🧠 Malloc Lab | 内存分配实验
- **Description**: Implement a dynamic memory allocator using explicit free lists with LIFO policy
- **描述**: 使用显式空闲链表和LIFO策略实现动态内存分配器
- **Topics**: Memory allocation, fragmentation, free list management
- **主题**: 内存分配、碎片整理、空闲链表管理
- **Key Skills**: Memory management, pointer manipulation
- **关键技能**: 内存管理、指针操作
- **Status**: ✅ Completed | 已完成
- **Implementation**: Explicit free list, LIFO policy, coalescing | 显式空闲链表，LIFO策略，合并机制


### 7. 🌐 Proxy Lab | 代理实验
- **Description**: Create a concurrent HTTP proxy
- **描述**: 并发HTTP代理
- **Topics**: Network and concurrency programming
- **主题**: 网络编程、并发编程
- **Key Skills**: Socket programming, thread synchronization
- **关键技能**: 套接字编程、线程同步
- **Status**: ✅ Completed | 已完成
- **Features**:
  - Concurrent request handling | 并发请求处理
  - I/O multiplexing techniques | I/O复用技术
  - HTTP/1.0 protocol support | HTTP/1.0协议支持


### 8. 🏗️ Arch Lab (Optional) | 架构实验（可选）
- **Description**: Not implemented in this repository
- **描述**: 本仓库中未实现
- **Status**: ❌ Optional architecture-related lab not included
- **状态**: 未包含的可选架构相关实验

---

## 🚀 Quick Start | 快速开始

### Prerequisites | 环境要求
```bash
# Ubuntu/Debian
sudo apt-get install gcc gdb make

# CentOS/RHEL
sudo yum install gcc gdb make
```

### Build & Run | 编译运行
```bash
# 以Proxy Lab为例
cd proxylab
make
./proxy 8080
```

### Testing | 测试
```bash
# 运行测试套件
make test
# 或使用提供的自动评分脚本
./driver.pl
```

---

## 🎯 Learning Outcomes | 学习收获

通过完成这些实验，我深入理解了：

- **计算机系统底层原理** - 从位操作到缓存层次结构
- **程序执行机制** - 汇编、链接、进程控制
- **系统安全** - 缓冲区溢出攻击与防御
- **并发编程** - 线程同步、信号处理
- **网络编程** - 套接字、HTTP协议
- **性能优化** - 缓存策略、内存分配效率



---

## 🔧 Technical Stack | 技术栈

- **Languages**: C, x86-64 Assembly
- **Tools**: GCC, GDB, Make, Valgrind
- **Platforms**: Linux, Unix-like systems
- **Concepts**: Computer Architecture, Operating Systems, Networking



---

## 🤝 Contributing | 贡献

Feel free to submit issues and pull requests for any improvements.

欢迎提交Issue和Pull Request来改进这个项目。

---

<div align="center">

**🌟 如果这个项目对你有帮助，请给个Star！ 🌟**

</div>

---
