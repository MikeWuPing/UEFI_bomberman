# Bomberman UEFI

**[English](README_en.md)** | 中文

经典炸弹人游戏的 UEFI Shell 移植版本。不需要操作系统，不需要图形库，一个 `.efi` 文件扔进 BIOS 直接玩。

> API 成本：3.3 元（Xiaomi Mimo V2.5 Pro via Claude Code）

## 效果展示

<!-- TODO: 替换为实际截图和动图 -->

![游戏截图](screenshots/gameplay.png)

## 特性

- 完整炸弹人游戏逻辑，约 2600 行纯 C 代码
- 15×15 地图，可破坏墙与不可破坏墙
- 炸弹放置、倒计时爆炸、四方向火焰扩散
- 三种敌人 AI：随机移动、穿墙幽灵、BFS 最短路径寻路
- 三种道具：增加炸弹数、扩大火焰范围、提升移动速度
- 32×32 TGA 精灵素材，alpha 通道透明混合
- 16×16 Consolas 位图字体
- UEFI GOP 双缓冲渲染，无画面撕裂
- 输出文件仅约 40KB

## 运行方式

### 使用 QEMU（推荐）

1. 安装 [QEMU](https://www.qemu.org/) 和 [OVMF](https://github.com/tianocore/edk2) 固件
2. 将编译产物 `Bomberman.efi` 和 `hda/sprites/` 目录复制到同一个文件夹
3. 启动 QEMU：

```bash
qemu-system-x86_64 \
  -bios OVMF.fd \
  -hda fat:rw:hda \
  -m 256M \
  -vga std
```

4. 在 UEFI Shell 中执行：

```
Shell> fs0:
FS0:\> Bomberman.efi
```

### 在真实硬件上运行

将 `Bomberman.efi` 和 `sprites/` 目录放入 UEFI 可启动的 FAT32 分区，通过 UEFI Shell 或 Boot Manager 启动。

## 操作说明

| 按键 | 功能 |
|------|------|
| W | 向上移动 |
| A | 向左移动 |
| S | 向下移动 |
| D | 向右移动 |
| E | 放置炸弹 |

## 从源码构建

### 环境要求

- [EDK II](https://github.com/tianocore/edk2) 源码
- Visual Studio 2019（Windows）或 GCC5（Linux）
- Python 3 + Pillow（仅在需要重新生成字体时）

### 构建步骤

```cmd
:: 设置环境变量（Windows CMD）
set WORKSPACE=D:\Work\Code\edk2
set EDK_TOOLS_PATH=%WORKSPACE%\BaseTools
set PACKAGES_PATH=D:\Work\Code\bomberman;%WORKSPACE%

:: 初始化 EDK II 环境
cd %WORKSPACE%
call edksetup.bat

:: 编译
build -p BombermanPkg\BombermanPkg.dsc -a X64 -t VS2019 -b DEBUG
```

编译产物：`Build/Bomberman/DEBUG_VS2019/X64/Bomberman.efi`

### 字体生成（可选）

如需重新生成位图字体：

```bash
python tools/gen_font.py --size 16x16 --font consola.ttf > tools/font16x16.h
```

## 项目结构

```
BombermanPkg/
├── BombermanPkg.dec               # EDK II 包声明
├── BombermanPkg.dsc               # EDK II 包描述
└── Application/Bomberman/
    ├── Bomberman.inf              # 模块定义
    ├── Main.c                     # 入口点与主循环
    ├── Game.c / Game.h            # 游戏管理器
    ├── Map.c / Map.h              # 地图系统（15×15 瓦片）
    ├── Player.c / Player.h        # 玩家逻辑
    ├── Bomb.c / Bomb.h            # 炸弹与火焰系统
    ├── Enemy.c / Enemy.h          # 敌人 AI（三种类型）
    ├── Gfx.c / Gfx.h              # GOP 图形引擎、精灵、字体
    └── Input.c / Input.h          # 键盘输入

hda/                               # QEMU 虚拟磁盘内容
├── EFI/BOOT/BOOTX64.EFI          # 自启动副本
├── Bomberman.efi                  # 游戏主程序
└── sprites/                       # TGA 精灵素材（14 个文件）

tools/
├── gen_font.py                    # Windows 字体位图提取工具
└── font16x16_generated.h          # 生成的字体数据
```

## 技术细节

### 图形渲染

使用 UEFI GOP（Graphics Output Protocol）的 `Blt()` 方法，采用双缓冲策略：所有绘图操作写入内存中的后缓冲区，每帧结束时一次性提交到屏幕。颜色格式为 BGRA（蓝-绿-红-透明）。

### 精灵加载

从 UEFI 文件系统读取 32 位 TGA（Targa）格式图片，解析 18 字节头部后直接读取 BGRA 像素数据。渲染时通过 alpha 通道实现透明混合（alpha >= 0x80 为不透明）。

### 输入处理

通过 `gST->ConIn->ReadKeyStroke()` 轮询键盘输入。UEFI 不支持连续按键状态查询，每帧轮询按键缓冲区实现近似实时的输入响应。

### 内存管理

使用 `gBS->AllocatePool()` / `gBS->FreePool()` 进行动态内存分配。无 C++ 运行时依赖，所有数据结构使用固定大小数组。

## 致谢

- 游戏逻辑参考自 [nukep/smb-vanilla-port](https://github.com/nukep/smb-vanilla-port) 的 UEFI 移植思路
- 精灵素材来自原始 SDL2 版炸弹人项目
- 使用 [Xiaomi Mimo V2.5 Pro](https://mimo.xiaomi.com/) 辅助开发

## 许可证

MIT License
