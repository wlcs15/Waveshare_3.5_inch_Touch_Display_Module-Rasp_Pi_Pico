#!/usr/bin/env python3
"""cppcheck on our quality sources. Ubuntu + Windows (cppcheck on PATH)."""
from __future__ import print_function

import os
import shutil
import subprocess
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
SRC = [
    os.path.join(ROOT, "lib", "quality"),
    os.path.join(ROOT, "tests"),
    os.path.join(ROOT, "host"),
]


def main():
    exe = shutil.which("cppcheck")
    if not exe:
        print("cppcheck not installed")
        return 1
    base = [
        exe,
        "--std=c11",
        "--inline-suppr",
        "--suppress=missingIncludeSystem",
    ]
    err = subprocess.call(base + ["--error-exitcode=1"] + SRC)
    if err != 0:
        return err
    subprocess.call(base + ["--enable=warning,style,performance"] + SRC)
    return 0


if __name__ == "__main__":
    sys.exit(main())
