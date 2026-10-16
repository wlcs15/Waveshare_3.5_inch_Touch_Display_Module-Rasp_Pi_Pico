#!/usr/bin/env bash
set -euo pipefail
exec "$(cd "$(dirname "$0")" && pwd)/py_launch.sh" run_coverage_target.py "$@"
