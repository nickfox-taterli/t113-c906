# T113i C906 小核固件

这是针对全志T113i芯片C906小核(RISC-V架构)的固件项目,使用FreeRTOS实时操作系统和OpenAMP实现与主核的通信.

## 项目简介

本项目提供了一个完整的C906小核固件框架,包括:

- FreeRTOS实时操作系统
- OpenAMP多核通信框架
- 硬件抽象层
- 中断控制器和定时器支持
- UART通信接口

## 项目架构

```
c906-fw/
├── arch/               # 架构相关代码
│   ├── includes/       # 头文件
│   └── riscv/          # RISC-V架构实现
├── core/               # 核心代码
│   ├── main.c          # 主程序
│   └── rsc_tab.c       # 资源表
├── hal/                # 硬件抽象层
│   ├── includes/       # HAL头文件
│   └── sources/        # HAL实现
├── FreeRTOS/           # FreeRTOS源码
├── openamp/            # OpenAMP库
└── build/              # 编译输出目录
```

## 硬件要求

- 全志T113i芯片
- C906 RISC-V核心(小核)

## 依赖项目

本固件需要配合以下项目使用:

### 内核
- [t113-linux](https://github.com/nickfox-taterli/t113-linux) - T113 Linux内核

### 其他工具
- [t113-utils](https://github.com/nickfox-taterli/t113-utils) - C906工具链和实用程序

## 编译环境

### 工具链要求

- Xuantie 900 GCC交叉编译工具链
- CMake 3.10或更高版本
- Make

### 工具链配置

编辑 `CMakeLists.txt` 文件中的工具链路径:

```cmake
set(TOOLCHAIN_PATH "/path/to/your/riscv64-elf-toolchain")
```

### 编译步骤

1. 创建编译目录:
```bash
mkdir build
cd build
```

2. 配置项目:
```bash
cmake ..
```

3. 编译:
```bash
make
```

4. 查看生成的文件:
```bash
ls -lh c906-fw.elf c906-fw.bin
```

### 编译输出

- `c906-fw.elf` - ELF格式的可执行文件
- `c906-fw.bin` - 二进制固件文件
- `c906-fw.map` - 链接映射文件
- `c906-fw.dis` - 反汇编文件(使用make disasm生成)

## 功能特性

### 已实现功能

- ✅ FreeRTOS实时操作系统
- ✅ OpenAMP多核通信
- ✅ UART串口通信(UART2,115200波特率)- pinmux需要在内核Hog.
- ✅ 中断控制器支持(PLIC和CLIC)
- ✅ 定时器支持
- ✅ 缓存管理(I-Cache和D-Cache)
- ✅ 时钟控制(CCU)

### OpenAMP通信

固件启动后会初始化OpenAMP平台,并创建主任务进行消息轮询:

```c
platform_rproc_init(&openamp_ctx);
// 主循环
while (1) {
    platform_poll(&openamp_ctx);
    vTaskDelay(pdMS_TO_TICKS(1));
}
```

## 系统配置

### FreeRTOS配置

核心配置文件位于 `core/FreeRTOSConfig.h`,可以调整:

### 内存配置

链接脚本位于 `c906.ld`,定义了内存布局和启动地址.

## 使用说明

### 加载固件

将编译好的 `c906-fw.elf` 塞到目标板 /lib/firmware/

启动小核

```
echo start > /sys/class/remoteproc/remoteproc0/state
```

### 调试输出

固件通过UART2输出调试信息,波特率115200:

```
[       0.000] main: Starting FreeRTOS on C906...
[       0.000] main: Main task created successfully!
[       0.001] main: OpenAMP: init begin
[       0.004] openamp: platform_rproc_init begin
[       0.009] openamp: metal_init ok
[       0.012] msgbox: init: local=2 remote=0 channel=0
[       0.017] msgbox: base: local=0x601f000 remote=0x3003000 local_n=0 remote_n=1 fifo_depth=8
[       0.025] msgbox: step: local RX IRQ enabled (base=0x601f000 n=0 p=0)
[       0.032] msgbox: step: local pending cleared
[       0.036] msgbox: step: initial poll done
[       0.041] msgbox: step: remote write IRQ cleared/disabled
[       0.046] msgbox: step: remote write threshold set to 1
[       0.051] msgbox: init done
[       0.054] openamp: msgbox ready (local=2 remote=0 local_n=0 remote_n=1 chan=0)
[       0.062] openamp: remoteproc_init ok (rproc=0x41230c88)
[       0.067] openamp: resource table @0x41230b90 len=148 version=1 entries=2
[       0.074] openamp: shm: da=0x414b0000 pa=0x414b0000 len=0x40000 flags=0x1
[       0.081] openamp: vring0: da=0x414ac000 align=64 num=128 notify=0
[       0.087] openamp: vring1: da=0x414ae000 align=64 num=128 notify=1
[       0.094] openamp: pa=0x41230b90 da=0x41230b90 size=0x94 va=0x41230b90 attr=0x0
[       0.101] openamp: pa=0x414b0000 da=0x414b0000 size=0x40000 va=0x414b0000 attr=0x1
[       0.109] openamp: resource table registered (pa=0x41230b90 da=0x41230b90)
[       0.116] openamp: remoteproc_config done
[       0.120] openamp: remoteproc_start done
[       0.124] openamp: pa=0x414ac000 da=0x414ac000 size=0xd46 va=0x414ac000 attr=0x0
[       0.132] openamp: pa=0x414ae000 da=0x414ae000 size=0xd46 va=0x414ae000 attr=0x0
[       0.139] openamp: virtio device created (notifyid=1 role=1)
[       0.145] openamp: rpmsg virtio ready (support_ns=1)
[       0.150] openamp: endpoint 'rpmsg-openamp-demo-channel' src=0x400 dst=0xffffffff created
[       0.158] openamp: RPMSG stack ready, waiting for host pings
[       0.164] main: OpenAMP init ok, echo ready.
```

## 许可证

本项目采用GPL-3.0许可证.详见 [LICENSE](LICENSE) 文件.

**重要条款:**

- 本项目可以用于商业用途
- 对本项目进行二次开发后,必须开源修改后的代码
- 作者不对代码的可靠性提供任何保证

## 免责声明

本代码按"原样"提供,不提供任何形式的明示或暗示保证.使用者需自行承担使用本代码的风险.作者不对因使用本代码导致的任何损失或损害承担责任.

有空可以到作者网站看看 [博客](https://www.taterli.com)