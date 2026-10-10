# scripts/ci

Emulator helpers shared with wlcs15/RR-Railcom-Pico2W-LCC-Waveshare-LCD2in-CAN
(identical copies of that repo's `scripts/ci/`):

- `check_arm_emulators.sh` - preflight for qemu-system-arm / Renode (FOUND/MISSING/DEFERRED)
- `run_renode_rp2040.sh` - Renode smoke (default emulator)
- `run_qemu_m33.sh` - QEMU smoke (experimental; Pico ELFs usually exit 134)

`Pico-ResTouch-LCD-X_X_Code/C/scripts/run_emulator_tests.*` calls these helpers.

The weekly multi-repo orchestrator `ci_arm_cycle.sh` is not copied here. It lives in
the Pico2W repo at `scripts/ci/ci_arm_cycle.sh` and builds this repo as path 2.
