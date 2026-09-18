#!/usr/bin/env bash
# Pick python / python3 (Windows Git Bash often has a Store stub as python3).
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
script="$1"
shift
cands=()
if command -v python >/dev/null 2>&1; then
  cands+=("python")
fi
if command -v python3 >/dev/null 2>&1; then
  cands+=("python3")
fi
if [[ ${#cands[@]} -eq 0 ]]; then
  echo "python not found"
  exit 1
fi
exec "${cands[0]}" -u "$root/scripts/$script" "$@"
