#!/usr/bin/env bash
# List whether required build + quality tools are on this Linux machine.
# Pico-ResTouch-LCD-3.5 / Pico W only. Does not install anything.
set -u

root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"
# Prefer workspace Debian/Renode wrappers when present (GrokBot-CI-ARM).
if [[ -d /workspace/tools/bin ]]; then
  export PATH="/workspace/tools/bin:${PATH}"
fi

if [[ -z "${CHECK_TOOLS_INNER:-}" ]]; then
    mkdir -p "$root/local"
    ts="$(date +%Y%m%d-%H%M%S)"
    host="$(hostname -s 2>/dev/null || hostname 2>/dev/null || echo unknown)"
    host="${host//[^A-Za-z0-9._-]/_}"
    log="$root/local/check_tools-${ts}-${host}.log"
    last="$root/local/check_tools-last.log"
    {
        echo "=== check_tools log (share this file with Grok; Grok CLI not required) ==="
        echo "file: $log"
        echo "time: $(date -Is 2>/dev/null || date)"
        echo "host: $(hostname 2>/dev/null || echo unknown)"
        echo "os: $(uname -a 2>/dev/null || echo unknown)"
        echo "user: ${USER:-unknown}"
        echo "repo: $root"
        echo "git: $(git -C "$root" describe --tags --always --dirty 2>/dev/null || echo n/a)"
        echo "python: $(command -v python3 2>/dev/null || command -v python 2>/dev/null || echo none)"
        echo
    } >"$log"
    set +e
    CHECK_TOOLS_INNER=1 "$0" "$@" 2>&1 | tee -a "$log"
    rc=${PIPESTATUS[0]}
    set -e
    cp -f "$log" "$last"
    echo
    echo "Share this file with Grok (no Grok CLI needed):"
    echo "  $log"
    echo "  $last"
    exit "$rc"
fi

missing_req=0
missing_opt=0

ok() { printf "  OK       %-18s %s\n" "$1" "$2"; }
fail() {
  printf "  MISSING  %-18s %s\n" "$1" "$2"
  missing_req=1
}
warn() {
  printf "  WARN     %-18s %s\n" "$1" "$2"
  missing_opt=1
}

have_cmd() { command -v "$1" >/dev/null 2>&1; }

py=""
if have_cmd python3; then
  py=python3
elif have_cmd python; then
  py=python
fi

pico_sdk="${PICO_SDK_PATH:-}"
if [[ ! -f "${pico_sdk}/pico_sdk_init.cmake" ]]; then
  pico_sdk="$HOME/.pico-sdk/sdk/2.2.0"
fi
toolchain="${PICO_TOOLCHAIN_PATH:-$HOME/.pico-sdk/toolchain/14_2_Rel1}"
picotool_bin=""
if have_cmd picotool; then
  picotool_bin="$(command -v picotool)"
elif [[ -x "$HOME/.pico-sdk/picotool/2.2.0-a4/picotool/picotool" ]]; then
  picotool_bin="$HOME/.pico-sdk/picotool/2.2.0-a4/picotool/picotool"
fi
armgcc="$toolchain/bin/arm-none-eabi-gcc"

echo "Required tools check (Linux)  repo: $root"
echo ""
echo "=== Host Unity + quality (required) ==="

if have_cmd git; then
  ok git "$(git --version 2>/dev/null | head -n1)"
else
  fail git "git clone"
fi

if [[ -n "$py" ]]; then
  ok python "$($py --version 2>&1) ($py)"
else
  fail python "Python 3 (python3)"
fi

if have_cmd cmake; then
  ok cmake "$(cmake --version 2>/dev/null | head -n1)"
else
  fail cmake "CMake 3.12+ (apt install cmake)"
fi

if have_cmd clang && have_cmd clang++; then
  ok clang "$(clang --version 2>/dev/null | head -n1)"
  ok "clang++" "$(clang++ --version 2>/dev/null | head -n1)"
else
  fail clang "LLVM Clang (apt install clang). Host tests + llvm-cov."
fi

if have_cmd ninja; then
  ok ninja "$(ninja --version 2>/dev/null)"
elif have_cmd make; then
  ok make "$(make --version 2>/dev/null | head -n1)"
else
  fail generator "Ninja or GNU make (apt install ninja-build)"
fi

unity="$root/third_party/Unity/src/unity.c"
if [[ -f "$unity" ]]; then
  ok Unity "third_party/Unity/src/unity.c"
else
  fail Unity "third_party/Unity/src/unity.c"
fi

echo ""
echo "=== Pico W firmware (required) ==="

if [[ -f "$pico_sdk/pico_sdk_init.cmake" ]]; then
  ok PicoSDK "$pico_sdk"
else
  fail PicoSDK "set PICO_SDK_PATH (expected $pico_sdk)"
fi

if [[ -x "$armgcc" ]]; then
  ok arm-gcc "$("$armgcc" --version 2>/dev/null | head -n1)"
elif have_cmd arm-none-eabi-gcc; then
  ok arm-gcc "$(arm-none-eabi-gcc --version 2>/dev/null | head -n1)"
else
  fail arm-gcc "Pico ARM GCC ($toolchain/bin/arm-none-eabi-gcc)"
fi

if [[ -n "$picotool_bin" ]]; then
  ok picotool "$("$picotool_bin" version 2>/dev/null | head -n1) ($picotool_bin)"
else
  fail picotool "~/.pico-sdk/picotool/.../picotool"
fi

echo ""
echo "=== Optional quality / coverage / on-target ==="

if have_cmd llvm-cov && have_cmd llvm-profdata; then
  ok llvm-cov "$(llvm-cov --version 2>/dev/null | head -n1)"
  ok llvm-profdata "host coverage (python scripts/run_coverage.py)"
else
  warn llvm-cov "apt install llvm  (scripts/run_coverage.sh)"
fi

if [[ -n "$py" ]] && "$py" -u "$root/scripts/run_lizard.py" --check >/dev/null 2>&1; then
  ok lizard "$("$py" -u "$root/scripts/run_lizard.py" --check 2>/dev/null | head -n1)"
else
  warn lizard "pipx install lizard  OR  python -m pip install lizard"
fi

if have_cmd clang-tidy; then
  ok clang-tidy "$(clang-tidy --version 2>/dev/null | head -n1)"
else
  warn clang-tidy "apt install clang-tidy  (scripts/run_clang_tidy.sh)"
fi

if have_cmd cppcheck; then
  ok cppcheck "$(cppcheck --version 2>/dev/null | head -n1)"
else
  warn cppcheck "apt install cppcheck  (scripts/run_cppcheck.sh)"
fi

if have_cmd oclint; then
  ok oclint "$(oclint --version 2>/dev/null | head -n1)"
else
  warn oclint "Linux only; skip if not installed (scripts/run_oclint.sh)"
fi

if have_cmd openocd; then
  ok openocd "$(openocd --version 2>&1 | head -n1)"
else
  warn openocd "Debug Probe reset / SWD (apt install openocd)"
fi

echo ""
echo "=== On-target emulators (required) ==="
echo "  (skip Espressif QEMU / simavr — other CI bots)"

emu_resolve() {
  local name="$1"
  if have_cmd "$name"; then
    command -v "$name"
    return 0
  fi
  if [[ -x "/workspace/tools/bin/$name" ]]; then
    echo "/workspace/tools/bin/$name"
    return 0
  fi
  return 1
}

qemu_arm=""
if qemu_arm=$(emu_resolve qemu-system-arm); then
  ok qemu-system-arm "$(qemu-system-arm --version 2>/dev/null | head -n1) ($qemu_arm)"
else
  fail qemu-system-arm "install qemu-system-arm (or put /workspace/tools/bin on PATH)"
fi

renode_bin=""
if renode_bin=$(emu_resolve renode); then
  ok renode "$(renode --version 2>/dev/null | head -n1) ($renode_bin)"
else
  fail renode "install Renode (or put /workspace/tools/bin on PATH)"
fi

qemu_a64=""
if qemu_a64=$(emu_resolve qemu-system-aarch64); then
  ok qemu-system-aarch64 "$(qemu-system-aarch64 --version 2>/dev/null | head -n1) ($qemu_a64) (optional)"
else
  warn qemu-system-aarch64 "optional companion softmmu; not required for Pico M0"
fi

echo ""
if [[ "$missing_req" -ne 0 ]]; then
  echo "Required tools are missing. See docs/REQUIRED_TOOLS.txt"
  exit 1
fi
if [[ "$missing_opt" -ne 0 ]]; then
  echo "Build tools OK. Optional items listed as WARN above."
  echo "Details: docs/REQUIRED_TOOLS.txt"
  exit 0
fi
echo "All required and optional tools found."
echo "Details: docs/REQUIRED_TOOLS.txt"
exit 0
