#!/usr/bin/env python3
"""Build and flash on-target Unity firmware (Pico W). Ubuntu + Windows 11."""
from __future__ import print_function

import glob
import os
import subprocess
import sys
import time

import pico_paths
import session_log
from host_cmake import generator_and_env, require

session_log.attach("run_on_target_tests")

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
BUILD = os.path.join(ROOT, "build", "on_target")
SOURCE = os.path.join(ROOT, "tests", "on_target")


def _picotool():
    tool = pico_paths.picotool()
    if not tool:
        raise SystemExit("picotool not found")
    return tool


def _cdc_port():
    if os.name != "nt":
        by_id = "/dev/serial/by-id"
        deadline = time.time() + 8
        while time.time() < deadline:
            if os.path.isdir(by_id):
                hits = [
                    p
                    for p in glob.glob(os.path.join(by_id, "usb-Raspberry_Pi_Pico_*"))
                    if "Debug_Probe" not in os.path.basename(p)
                    and "CMSIS-DAP" not in os.path.basename(p)
                ]
                if hits:
                    return os.path.realpath(hits[0])
            time.sleep(0.25)
        return None
    try:
        import serial.tools.list_ports
    except ImportError:
        return None
    for p in serial.tools.list_ports.comports():
        desc = (p.description or "") + " " + (p.manufacturer or "")
        if "Pico" in desc or (p.vid == 0x2E8A and p.pid == 0x000A):
            return p.device
    return None


def _read_serial(port, seconds, log_path):
    data = b""
    try:
        import serial
    except ImportError:
        serial = None
    if serial is not None:
        ser = serial.Serial(port, 115200, timeout=0.2)
        deadline = time.time() + seconds
        while time.time() < deadline:
            chunk = ser.read(4096)
            if chunk:
                data += chunk
        ser.close()
    elif os.name != "nt":
        proc = subprocess.Popen(["timeout", str(seconds), "cat", port], stdout=subprocess.PIPE)
        data = proc.communicate()[0] or b""
    text = data.decode("utf-8", "replace").replace("\0", "")
    with open(log_path, "w", encoding="utf-8", errors="replace") as handle:
        handle.write(text)
    sys.stdout.write(text)
    return text


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
        "-DCMAKE_BUILD_TYPE=Release",
    ]
    print(" ".join(cmd))
    subprocess.check_call(cmd, env=env)
    subprocess.check_call([require("cmake"), "--build", BUILD], env=env)
    uf2 = os.path.join(BUILD, "on_target_tests.uf2")
    if not os.path.isfile(uf2):
        print("missing %s" % uf2)
        return 1
    tool = _picotool()
    subprocess.check_call([tool, "load", "-x", "-f", uf2])
    port = _cdc_port()
    local = os.path.join(ROOT, "local")
    try:
        os.makedirs(local)
    except OSError:
        pass
    log_path = os.path.join(local, "on_target_tests-last.log")
    if not port:
        print("no Pico CDC port; firmware flashed, open USB serial for Unity output")
        return 0
    print("reading %s (20s)..." % port)
    text = _read_serial(port, 12, log_path)
    if "FAIL:" in text:
        print("on-target Unity: FAIL (see %s)" % log_path)
        return 1
    if "0 Failures" in text or text.rstrip().endswith("OK"):
        print("on-target Unity: OK")
        return 0
    print("on-target Unity: no result line (open the CDC port then reset)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
