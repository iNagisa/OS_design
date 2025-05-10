Limine C 模板项目
本仓库将演示如何使用 Limine 在 C 语言中设置基础内核。

如何使用本项目？
任何 make 命令都依赖于 GNU make (gmake)，且需要使用该工具运行。在大多数 GNU/Linux 发行版中通常直接使用 make 命令，在非 GNU 系统上可能需要使用 gmake。
建议在标准的类 UNIX 系统上构建本项目，使用支持交叉编译的 Clang/LLVM 工具链。

此外：

使用 make all 生成 ISO 镜像需要 xorriso
使用 make all-hdd 生成 HDD/USB 镜像需要：
sgdisk（通常来自 gdisk 或 gptfdisk 软件包）/ mtools

架构目标
ARCH 编译变量决定目标架构：默认架构为 x86_64

Makefile 目标
make all	编译内核（位于 kernel/ 目录）并生成可启动的 ISO 镜像
make all-hdd	编译内核并生成适合刷写到USB设备或硬盘/SSD的原始镜像
make run	构建内核和可启动 ISO（等效于 make all）后使用 QEMU 运行（需已安装）
make run-hdd	构建内核和原始 HDD 镜像（等效于 make all-hdd）后使用 QEMU 运行
x86_64 特殊选项
make run-bios	使用 SeaBIOS 固件启动（默认）替代 OVMF
make run-hdd-bios	HDD 镜像版本使用 SeaBIOS 启动
