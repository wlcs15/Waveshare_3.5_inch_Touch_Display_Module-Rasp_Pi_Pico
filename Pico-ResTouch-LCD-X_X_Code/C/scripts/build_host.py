#!/usr/bin/env python3
"""Configure and build host Unity tests with Clang + CMake."""
from __future__ import print_function

import sys

import session_log
from host_cmake import HOST_BUILD, cmake_build, cmake_configure, tests_exe

session_log.attach("build_host")


def main():
    env = cmake_configure(HOST_BUILD, coverage=False)
    cmake_build(HOST_BUILD, env)
    print("host binaries in %s" % HOST_BUILD)
    print("tests: %s" % tests_exe(HOST_BUILD))
    return 0


if __name__ == "__main__":
    sys.exit(main())
