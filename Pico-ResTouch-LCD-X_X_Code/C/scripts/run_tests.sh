#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
if command -v python3 >/dev/null 2>&1; then
  py=python3
elif command -v python >/dev/null 2>&1; then
  py=python
else
  echo "python3 not found"
  exit 1
fi
"$py" "$root/scripts/build_host.py"
if [[ -x "$root/build/host/pico_lcd_tests" ]]; then
  exec "$root/build/host/pico_lcd_tests"
fi
if [[ -x "$root/build/host/pico_lcd_tests.exe" ]]; then
  exec "$root/build/host/pico_lcd_tests.exe"
fi
echo "pico_lcd_tests not built in $root/build/host"
exit 1
