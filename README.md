# 目的
 熟悉基础的网络编程；熟悉 `HTTP` 协议；熟悉 `WebServer`；熟悉 `Linux` 环境编程；
  
# 要求
  语言：`C语言`（`gcc`）

  环境：`Linux`

  ***

# part1
## 要求
* 实现一个简单的基于`tcp`的`WebServer`;
* 需要有`Makefile`，在代码仓库下，直接执行`make`可以生成一个可执行程序`server`；
* 命令行参数`./server <root-path> <port>`；
* 支持`html`、`css`、`javascript`、`image/jpeg`，设置合理的`minetype`；
* 使用`多进程`实现并发；
