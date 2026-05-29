# Bomberman UEFI

English | **[中文](README.md)**

A complete Bomberman game ported to the UEFI Shell environment. No OS required, no graphics library needed — just drop a single `.efi` file into a BIOS shell and play.

> API cost: ¥3.3 CNY (~$0.45 USD) using Xiaomi Mimo V2.5 Pro via Claude Code

## Demo

<!-- TODO: Replace with actual screenshots and GIFs -->

![Gameplay Screenshot](screenshots/gameplay.png)

## Features

- Full Bomberman game logic in ~2600 lines of pure C
- 15×15 tile map with breakable and unbreakable walls
- Bomb placement, timed detonation, 4-direction flame spread
- Three enemy AI types: random walk, wall-phasing ghost, BFS pathfinding
- Three power-ups: extra bombs, extended flames, speed boost
- 32×32 TGA sprite assets with alpha channel transparency
- 16×16 Consolas bitmap font
- UEFI GOP double-buffered rendering, no screen tearing
- Output binary is only ~40KB

## Running

### Using QEMU (Recommended)

1. Install [QEMU](https://www.qemu.org/) and [OVMF](https://github.com/tianocore/edk2) firmware
2. Place the compiled `Bomberman.efi` and the `hda/sprites/` directory in the same folder
3. Launch QEMU:

```bash
qemu-system-x86_64 \
  -bios OVMF.fd \
  -hda fat:rw:hda \
  -m 256M \
  -vga std
```

4. In the UEFI Shell, run:

```
Shell> fs0:
FS0:\> Bomberman.efi
```

### Running on Real Hardware

Place `Bomberman.efi` and the `sprites/` directory on a FAT32 UEFI-bootable partition, then launch from UEFI Shell or Boot Manager.

## Controls

| Key | Action |
|-----|--------|
| W | Move up |
| A | Move left |
| S | Move down |
| D | Move right |
| E | Place bomb |

## Building from Source

### Prerequisites

- [EDK II](https://github.com/tianocore/edk2) source code
- Visual Studio 2019 (Windows) or GCC5 (Linux)
- Python 3 + Pillow (only needed for font regeneration)

### Build Steps

```cmd
:: Set environment variables (Windows CMD)
set WORKSPACE=D:\Work\Code\edk2
set EDK_TOOLS_PATH=%WORKSPACE%\BaseTools
set PACKAGES_PATH=D:\Work\Code\bomberman;%WORKSPACE%

:: Initialize EDK II environment
cd %WORKSPACE%
call edksetup.bat

:: Build
build -p BombermanPkg\BombermanPkg.dsc -a X64 -t VS2019 -b DEBUG
```

Output: `Build/Bomberman/DEBUG_VS2019/X64/Bomberman.efi`

### Font Generation (Optional)

To regenerate the bitmap font:

```bash
python tools/gen_font.py --size 16x16 --font consola.ttf > tools/font16x16.h
```

## Project Structure

```
BombermanPkg/
├── BombermanPkg.dec               # EDK II package declaration
├── BombermanPkg.dsc               # EDK II package description
└── Application/Bomberman/
    ├── Bomberman.inf              # Module definition
    ├── Main.c                     # Entry point and main loop
    ├── Game.c / Game.h            # Game manager
    ├── Map.c / Map.h              # Map system (15×15 tiles)
    ├── Player.c / Player.h        # Player logic
    ├── Bomb.c / Bomb.h            # Bomb and flame system
    ├── Enemy.c / Enemy.h          # Enemy AI (three types)
    ├── Gfx.c / Gfx.h              # GOP graphics engine, sprites, font
    └── Input.c / Input.h          # Keyboard input

hda/                               # QEMU virtual disk contents
├── EFI/BOOT/BOOTX64.EFI          # Auto-boot copy
├── Bomberman.efi                  # Game binary
└── sprites/                       # TGA sprite assets (14 files)

tools/
├── gen_font.py                    # Windows font bitmap extractor
└── font16x16_generated.h          # Generated font data
```

## Technical Details

### Graphics Rendering

Uses the UEFI GOP (Graphics Output Protocol) `Blt()` method with a double-buffered approach: all drawing operations write to a back buffer in memory, which is flushed to the screen once per frame. Color format is BGRA (Blue-Green-Red-Alpha).

### Sprite Loading

Reads 32-bit TGA (Targa) image files from the UEFI filesystem. After parsing the 18-byte header, BGRA pixel data is read directly. Transparency is achieved via the alpha channel (alpha >= 0x80 is opaque).

### Input Handling

Polls keyboard input via `gST->ConIn->ReadKeyStroke()`. Since UEFI does not support continuous key state queries, the input buffer is polled each frame for near-realtime response.

### Memory Management

Uses `gBS->AllocatePool()` / `gBS->FreePool()` for dynamic memory allocation. No C++ runtime dependency — all data structures use fixed-size arrays.

## Credits

- Game logic ported with reference to the SDL2 Bomberman project design
- Sprite assets from the original SDL2 Bomberman project
- Built with assistance from [Xiaomi Mimo V2.5 Pro](https://mimo.xiaomi.com/)

## License

MIT License
