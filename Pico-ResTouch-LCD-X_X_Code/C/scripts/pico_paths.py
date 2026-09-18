#!/usr/bin/env python3
"""Pico SDK / toolchain / picotool locations on Ubuntu and Windows 11."""
from __future__ import print_function

import os
import shutil


def home():
    return os.path.expanduser("~")


def sdk_root():
    env = os.environ.get("PICO_SDK_PATH")
    if env and os.path.isfile(os.path.join(env, "pico_sdk_init.cmake")):
        return env
    cand = os.path.join(home(), ".pico-sdk", "sdk", "2.2.0")
    if os.path.isfile(os.path.join(cand, "pico_sdk_init.cmake")):
        return cand
    return env or cand


def toolchain_root():
    env = os.environ.get("PICO_TOOLCHAIN_PATH")
    if env and os.path.isdir(env):
        return env
    return os.path.join(home(), ".pico-sdk", "toolchain", "14_2_Rel1")


def arm_gcc():
    gcc = os.path.join(toolchain_root(), "bin", "arm-none-eabi-gcc")
    if os.name == "nt":
        gcc += ".exe"
    if os.path.isfile(gcc):
        return gcc
    return shutil.which("arm-none-eabi-gcc")


def picotool():
    env = os.environ.get("PICOTOOL")
    if env and os.path.isfile(env):
        return env
    found = shutil.which("picotool")
    if found:
        return found
    name = "picotool.exe" if os.name == "nt" else "picotool"
    cand = os.path.join(home(), ".pico-sdk", "picotool", "2.2.0-a4", "picotool", name)
    if os.path.isfile(cand):
        return cand
    return None
