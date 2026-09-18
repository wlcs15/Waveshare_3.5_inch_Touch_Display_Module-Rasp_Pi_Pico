#!/usr/bin/env python3
"""Host clang-tidy fail gate for lib/quality + host tests. Migrated from A5.02."""
from __future__ import print_function

import os
import shutil
import subprocess
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
WAE = "clang-diagnostic-error,clang-analyzer-*"
CONFIG = os.path.join(ROOT, ".clang-tidy")
COMPILE_DB = os.path.join(ROOT, "build", "host")
SOURCES = [
    os.path.join(ROOT, "lib", "quality", "bmp_policy.c"),
    os.path.join(ROOT, "tests", "test_bmp_policy.c"),
]


def find_clang_tidy():
    names = ("clang-tidy", "clang-tidy.exe")
    for name in names:
        found = shutil.which(name)
        if found:
            return found
    extras = ["/usr/bin/clang-tidy", "/usr/lib/llvm-18/bin/clang-tidy"]
    if os.name == "nt":
        pf = os.environ.get("ProgramFiles", r"C:\Program Files")
        pf86 = os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)")
        extras = [
            os.path.join(pf, "LLVM", "bin", "clang-tidy.exe"),
            os.path.join(pf86, "LLVM", "bin", "clang-tidy.exe"),
        ]
    for path in extras:
        if os.path.isfile(path):
            return path
    return None


def main():
    tidy = find_clang_tidy()
    if not tidy:
        print("clang-tidy not installed")
        return 1
    os.chdir(ROOT)
    rc = subprocess.call([sys.executable, "-u", os.path.join(ROOT, "scripts", "build_host.py")])
    if rc != 0:
        return rc
    db = os.path.join(COMPILE_DB, "compile_commands.json")
    if not os.path.isfile(db):
        print("missing compile_commands.json")
        return 1
    cmd = [
        tidy,
        "--config-file=" + CONFIG,
        "--warnings-as-errors=" + WAE,
        "-p",
        COMPILE_DB,
    ] + SOURCES
    rc = subprocess.call(cmd)
    if rc != 0:
        return rc
    print("OK: clang-tidy host gate (lib/quality + tests/; no vendor LCD)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
