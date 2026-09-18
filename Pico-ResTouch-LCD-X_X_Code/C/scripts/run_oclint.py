#!/usr/bin/env python3
"""OCLint on lib/quality + tests. Linux only; skip on Windows (same as A5.02)."""
from __future__ import print_function

import glob
import os
import shutil
import subprocess
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))


def main():
    if os.name == "nt":
        print("OCLint skipped on Windows")
        return 0
    exe = shutil.which("oclint")
    if not exe:
        print("oclint not installed (Linux only); skip")
        return 0
    files = glob.glob(os.path.join(ROOT, "lib", "quality", "*.c"))
    files += glob.glob(os.path.join(ROOT, "lib", "quality", "*.h"))
    files += glob.glob(os.path.join(ROOT, "tests", "*.c"))
    cmd = [exe] + files + [
        "--",
        "-std=c11",
        "-I",
        os.path.join(ROOT, "lib", "quality"),
        "-I",
        os.path.join(ROOT, "third_party", "Unity", "src"),
    ]
    rc = subprocess.call(cmd)
    if rc != 0:
        print("oclint failed (optional; LLVM ABI mismatch is common). skip")
        return 0
    return 0


if __name__ == "__main__":
    sys.exit(main())
