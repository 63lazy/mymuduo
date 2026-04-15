# mymuduo - 高性能 C++ 网络库

基于 C++11 重构实现的异步网络底层框架，核心思想借鉴 Muduo 库。本项目旨在通过 Reactor 模型实现非阻塞 IO + 多线程并发，提供工业级高性能的网络通信能力。

## 核心特性
- **模型架构**：采用 Multiple Reactors + Thread Pool (One Loop Per Thread) 模型。主 Reactor 负责连接分发，子 Reactor 负责具体的 IO 事件处理。
- **IO 多路复用**：封装 Epoll 接口，默认采用 LT 触发模式（可扩展 ET），利用 IO 多路复用处理万级并发。
- **线程安全**：深度实践 RAII 机制，利用 `shared_from_this` 与 `weak_ptr` 解决了多线程环境下 TCP 连接对象生命周期管理的竞态条件问题。
- **内存管理**：实现自动增长的应用层 Buffer 缓冲区，利用 `readv` 结合栈空间缓存减少系统调用，优化堆内存分配。
- **定时器系统**：基于 `timerfd` 实现的高性能定时器，支持 $O(logN)$ 复杂度的超时连接清理。
- **异步日志**：(正在迁移) 具备双缓冲机制的异步日志系统，确保 IO 线程不被磁盘写入阻塞。

## 开发进度 (Roadmap)
- [x] 基于 Epoll 的 Reactor 核心事件循环
- [x] 多线程 EventLoopThreadPool 实现
- [x] 应用层 Buffer 动态缓冲区设计
- [x] 定时器与异步唤醒机制 (eventfd)
- [ ] **[进行中]** 基于 TLV (Type-Length-Value) 格式的应用层通信协议
- [ ] **[计划中]** 集成 Protobuf 序列化支持

## 编译与构建
本项目采用 CMake 进行构建：
```bash
/bin/bash autobuild.sh

##项目说明与当前状态 (Current Status)

本项目目前处于 **活跃开发阶段 (Active Development)**，核心网络库逻辑（Reactor 模型、多线程、Buffer 管理）已完成并经过本地性能测试。

**关于应用层协议解析：**
目前正在从零实现一套基于 **TLV (Type-Length-Value)** 格式的二进制编解码器，以替代传统的文本解析。
*   **进度说明**：已完成 Header 的魔数校验与长度字段解析，正在调试针对大包传输的粘包处理逻辑。
*   **设计目标**：旨在后续集成 Protobuf 序列化框架，为高性能即时通讯（IM）场景提供支持。