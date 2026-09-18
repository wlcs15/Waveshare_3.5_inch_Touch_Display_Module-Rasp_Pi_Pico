#!/usr/bin/env python3
"""On-target --coverage firmware build + host llvm-cov of the same TUs."""
from __future__ import print_function

import os
import subprocess
import sys

import pico_paths
import session_log
from host_cmake import generator_and_env, require

session_log.attach("run_coverage_target")

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
BUILD = os.path.join(ROOT, "build", "on_target-coverage")
SOURCE = os.path.join(ROOT, "tests", "on_target")


def main():
    require("cmake")
    sdk = pico_paths.sdk_root()
    tc = pico_paths.toolchain_root()
    os.environ["PICO_SDK_PATH"] = sdk
    os.environ["PICO_TOOLCHAIN_PATH"] = tc
    gen, env = generator_and_env()
    env = dict(env)
    env["PICO_SDK_PATH"] = sdk
    env["PICO_TOOLCHAIN_PATH"] = tc
    os.makedirs(BUILD, exist_ok=True)
    cmd = [
        require("cmake"),
        "-S",
        SOURCE,
        "-B",
        BUILD,
        "-G",
        gen,
        "-DPICO_BOARD=pico_w",
        "-DCMAKE_BUILD_TYPE=Debug",
        "-DPICO_TEST_COVERAGE=ON",
    ]
    print(" ".join(cmd))
    subprocess.check_call(cmd, env=env)
    subprocess.check_call([require("cmake"), "--build", BUILD], env=env)
    print("gcno notes under %s (no .gcda on device without a FS)." % BUILD)
    print("Host llvm-cov of the same lib/quality sources:")
    rc = subprocess.call([sys.executable, "-u", os.path.join(ROOT, "scripts", "run_coverage.py")])
    if rc != 0:
        return rc
    print("on-target coverage build OK; host report is recorded line coverage for bmp_policy.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
