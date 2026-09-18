#!/usr/bin/env bash
set -euo pipefail
exec "$(cd "$(dirname "$0")" && pwd)/py_launch.sh" run_on_target_tests.py "$@"
