# Waveshare 3.5" Touch Display — Raspberry Pi Pico W

C firmware for the **Waveshare Pico-ResTouch-LCD-3.5** (ILI9488 + XPT2046) on a **Raspberry Pi Pico W**. This tree started from Waveshare’s Pico-ResTouch sample and adds Pico SDK 2.2, `pico_w` as the board, USB debug, and the same host quality scripts used on the OwlThree LCC boards **A5.01** (D1 R32 display) and **A5.02** (Mega host tests).

Work from:

```text
Pico-ResTouch-LCD-X_X_Code/C
```

## Hardware

Seat the Pico W so USB lines up with the USB mark on the HAT.

| Function | GPIO |
|---|---|
| LCD_DC / LCD_CS | 8 / 9 |
| SPI CLK / MOSI / MISO | 10 / 11 / 12 (`spi1`) |
| Backlight / Reset | 13 / 15 |
| Touch CS / IRQ | 16 / 17 |
| SD CS | 22 |

CYW43439 on Pico W uses GPIO 23–25 and 29; the display does not.

**SD card:** optional FAT/FAT32 slideshow. The demo wants **24-bit** Windows BMPs whose header size matches the current scan dir (default **320×480**, not 480×320). 8.3 names, extension `BMP`, files in the volume root. After one pass the firmware goes to the touch UI.

## Firmware build (Ubuntu x86 or Windows 11)

Needs Pico SDK 2.2, ARM GNU 14.2, CMake, Ninja, picotool (Raspberry Pi VS Code Pico extension is enough).

Ubuntu:

```bash
cd Pico-ResTouch-LCD-X_X_Code/C
export PICO_SDK_PATH="${PICO_SDK_PATH:-$HOME/.pico-sdk/sdk/2.2.0}"
export PICO_TOOLCHAIN_PATH="${PICO_TOOLCHAIN_PATH:-$HOME/.pico-sdk/toolchain/14_2_Rel1}"
cmake -S . -B build -DPICO_BOARD=pico_w -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build build
# flash LCD demo
picotool load -x -f build/main.uf2
```

Windows 11 (PowerShell, same SDK layout under `%USERPROFILE%\.pico-sdk`):

```powershell
cd Pico-ResTouch-LCD-X_X_Code\C
cmake -S . -B build -DPICO_BOARD=pico_w -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build build
picotool load -x -f build\main.uf2
```

USB serial for `printf` is the Pico CDC port, not the Debug Probe UART.

## Quality scripts

Every quality entry has a **Python core** plus **`.sh` (Ubuntu / Git Bash)** and **`.ps1` (Windows 11)** launchers. Run them from `Pico-ResTouch-LCD-X_X_Code/C`. They log under `local/` (gitignored).

Ubuntu:

```bash
bash scripts/check_tools.sh
```

Windows 11:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts\check_tools.ps1
```

| Script | Purpose |
|---|---|
| `check_tools` | Lists whether **this** tree can build and run quality (git, Python 3, CMake, Clang, Ninja, Unity, Pico SDK, ARM GCC, picotool). Optional: llvm-cov, lizard, clang-tidy, cppcheck, oclint, openocd. Does **not** install anything. No Arduino-cli / Wi-Fi wrap. |
| `build_host` | Clang + CMake host Unity binary `build/host/pico_lcd_tests`. |
| `run_tests` | Host Unity tests for `lib/quality/bmp_policy.c` (slideshow scan, BMP header, 24 bpp, 320×480, `.BMP` names). |
| `run_coverage` | Host **llvm-cov** (`-fprofile-instr-generate`). HTML: `build/host-coverage/coverage/index.html`. |
| `run_coverage_target` | On-device GCC `--coverage` (`.gcno` only; Pico has no FS for `.gcda`) plus the host llvm-cov report of the same TUs. |
| `run_on_target_tests` | Builds `tests/on_target`, flashes `on_target_tests.uf2`, reads Unity over USB CDC. **Replaces the LCD demo** until you reflash `build/main.uf2`. |
| `run_lizard` | Cyclomatic complexity. Fails if any function in `lib/quality`, `tests`, or `host` has **CCN > 10**. Vendor LCD/FatFs is not scanned. |
| `run_clang_tidy` | Host clang-tidy on `lib/quality` + `tests` only (not `lib/lcd`, `lib/fatfs`, `lib/font`). |
| `run_cppcheck` | cppcheck C11 errors fail; style/performance are reports. |
| `run_oclint` | OCLint on our C sources. **Skipped on Windows** (same as A5.02). |

Tool versions and install hints: `Pico-ResTouch-LCD-X_X_Code/C/docs/REQUIRED_TOOLS.txt`.

Host tests need **LLVM Clang**, not MinGW. Coverage needs `llvm-cov` and `llvm-profdata` on `PATH` (`C:\Program Files\LLVM\bin` on Windows).

## Clang-tidy

`scripts/run_clang_tidy` checks **`lib/quality`** and **`tests` only**. Vendor **`lib/lcd`**, **`lib/fatfs`**, and **`lib/font`** are not on the file list and are outside `HeaderFilterRegex`.

Checks are `clang-diagnostic-*` and `clang-analyzer-*` only (no `google-*` / `cert-*`). Call sites use `memset_s` from `lib/quality/string_s.c`. `bmp_policy_pad_8_3` takes `out_len` and requires at least 12 bytes.
