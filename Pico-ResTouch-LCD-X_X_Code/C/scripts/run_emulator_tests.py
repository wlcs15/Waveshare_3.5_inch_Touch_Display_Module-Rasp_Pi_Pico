#!/usr/bin/env python3
"""Run on-target Unity firmware under Renode (RP2040 / Pico W). Does NOT flash.

Wraps the same tests/on_target Unity image that scripts/run_on_target_tests.*
builds and flashes, but loads the ELF in Renode instead of picotool.

Usage:
  bash scripts/run_emulator_tests.sh
  bash scripts/run_emulator_tests.sh --elf build/on_target/on_target_tests.elf
  bash scripts/run_emulator_tests.sh --elf build/main.elf
  powershell -NoProfile -ExecutionPolicy Bypass -File scripts\\run_emulator_tests.ps1

Default emulator: Renode RP2040 smoke (matches GrokBot-CI-ARM for this tree).
Optional --qemu uses qemu-system-arm (not the primary path for RP2040 here).

Exit: 0 clean, 124 timeout→smoke OK, 2 missing tools/ELF.
Prefer PATH with /workspace/tools/bin (source /workspace/env/emulators.sh).
"""
from __future__ import print_function

import argparse
import os
import shutil
import subprocess
import sys
import tempfile

import pico_paths
from host_cmake import generator_and_env, require

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
SOURCE = os.path.join(ROOT, "tests", "on_target")
BUILD = os.path.join(ROOT, "build", "on_target")
_WORKSPACE_BIN = "/workspace/tools/bin"
_CI_RENODE = "/workspace/tools/ci/run_renode_rp2040.sh"
_CI_QEMU = "/workspace/tools/ci/run_qemu_m33.sh"


def _ensure_workspace_path():
    if os.path.isdir(_WORKSPACE_BIN):
        os.environ["PATH"] = _WORKSPACE_BIN + os.pathsep + os.environ.get("PATH", "")


def _which(name):
    found = shutil.which(name)
    if found:
        return found
    if os.path.isdir(_WORKSPACE_BIN):
        cand = os.path.join(_WORKSPACE_BIN, name)
        if os.path.isfile(cand) and os.access(cand, os.X_OK):
            return cand
    return None


def _default_elf():
    for path in (
        os.path.join(BUILD, "on_target_tests.elf"),
        os.path.join(ROOT, "build", "main.elf"),
    ):
        if os.path.isfile(path):
            return path
    return os.path.join(BUILD, "on_target_tests.elf")


def _build_on_target():
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
        "-DCMAKE_BUILD_TYPE=Release",
    ]
    print("EMU configure on_target Unity")
    print(" ".join(cmd))
    subprocess.check_call(cmd, env=env)
    subprocess.check_call([require("cmake"), "--build", BUILD], env=env)
    elf = os.path.join(BUILD, "on_target_tests.elf")
    if not os.path.isfile(elf):
        raise SystemExit("EMU missing %s after build" % elf)
    return elf


def _run_ci_helper(helper, elf, timeout_sec, log_base):
    if os.path.isfile(helper):
        print("EMU using CI helper: %s" % helper)
        return subprocess.call(["bash", helper, elf, str(timeout_sec), log_base])
    return None


def _run_renode(elf, timeout_sec, log_base):
    if not _which("renode"):
        print(
            "EMU missing renode "
            "(source /workspace/env/emulators.sh or install Renode)",
            file=sys.stderr,
        )
        return 2
    rc = _run_ci_helper(_CI_RENODE, elf, timeout_sec, log_base)
    if rc is not None:
        return rc
    abs_elf = os.path.realpath(elf)
    out = log_base + ".renode.out"
    resc = tempfile.NamedTemporaryFile("w", suffix=".resc", delete=False)
    try:
        resc.write(
            """using sysbus
mach create "rp2040_smoke"
machine LoadPlatformDescriptionFromString """
            '"""\n'
            """cpu: CPU.CortexM @ sysbus
    cpuType: \\"cortex-m0\\"
    nvic: nvic
nvic: IRQControllers.NVIC @ sysbus 0xE000E000
    -> cpu@0
flash: Memory.MappedMemory @ sysbus 0x10000000
    size: 0x00200000
sram: Memory.MappedMemory @ sysbus 0x20000000
    size: 0x00042000
"""
            '"""\n'
            "sysbus LoadELF @%s\n"
            "cpu PC 0x10000000\n"
            "start\n"
            'emulation RunFor "00:00:02"\n' % abs_elf
        )
        resc.close()
        cmd = [
            _which("renode"),
            "--disable-xwt",
            "--console",
            "-e",
            "include @%s; quit" % resc.name,
        ]
        print("EMU renode timeout=%ss elf=%s" % (timeout_sec, abs_elf))
        try:
            with open(out, "wb") as handle:
                proc = subprocess.run(
                    cmd,
                    stdin=subprocess.DEVNULL,
                    stdout=handle,
                    stderr=subprocess.STDOUT,
                    timeout=timeout_sec,
                )
            print("EMU renode exit=%s (see %s)" % (proc.returncode, out))
            return proc.returncode
        except subprocess.TimeoutExpired:
            print("EMU renode timeout (%ss) — smoke OK" % timeout_sec)
            return 124
    finally:
        try:
            os.unlink(resc.name)
        except OSError:
            pass


def _run_qemu(elf, timeout_sec, log_base):
    if not _which("qemu-system-arm"):
        print("EMU missing qemu-system-arm", file=sys.stderr)
        return 2
    rc = _run_ci_helper(_CI_QEMU, elf, timeout_sec, log_base)
    if rc is not None:
        return rc
    print("EMU --qemu path expects CI helper %s" % _CI_QEMU, file=sys.stderr)
    return 2


def main(argv=None):
    _ensure_workspace_path()
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--elf", default="", help="ELF to load (default: build/on_target/on_target_tests.elf)")
    ap.add_argument("--timeout", type=int, default=90)
    ap.add_argument("--qemu", action="store_true", help="Use qemu-system-arm instead of Renode")
    ap.add_argument("--no-build", action="store_true")
    ap.add_argument("--log", default="")
    args = ap.parse_args(argv)

    elf = args.elf or _default_elf()
    if not os.path.isfile(elf):
        if args.no_build:
            print("EMU missing ELF: %s" % elf, file=sys.stderr)
            return 2
        try:
            elf = _build_on_target()
        except Exception as exc:
            print("EMU on_target build failed: %s" % exc, file=sys.stderr)
            return 1

    log_base = args.log or os.path.join(ROOT, "build", "emulator-smoke")
    parent = os.path.dirname(log_base)
    if parent and not os.path.isdir(parent):
        os.makedirs(parent)

    print("EMU elf=%s" % elf)
    print("EMU note: does not flash; hardware path remains scripts/run_on_target_tests.*")
    print("EMU note: host Unity remains scripts/run_tests.*")
    if args.qemu:
        rc = _run_qemu(elf, args.timeout, log_base)
    else:
        rc = _run_renode(elf, args.timeout, log_base)

    if rc in (0, 124):
        print("EMU smoke RESULT: PASS (exit=%s)" % rc)
        return 0
    print("EMU smoke RESULT: FAIL (exit=%s)" % rc)
    return rc if rc else 1


if __name__ == "__main__":
    sys.exit(main())
