"""
PlatformIO pre-build script: weaken_libnet80211.py
===================================================
Automatically ensures ieee80211_raw_frame_sanity_check is a WEAK symbol in
libnet80211.a so that our override in Deauther.cpp can intercept it.

Safety guarantees:
  1. Detects whether the symbol is already weak — avoids redundant patching.
  2. Creates a .bak backup if one does not already exist.
  3. Logs the framework version and patch state.
  4. Does nothing if objcopy or the library cannot be found.
"""

import os
import subprocess
import shutil

Import("env")

def weaken_sanity_check(source, target, env):
    platform_dir = env.PioPlatform().get_package_dir("framework-arduinoespressif32")
    if not platform_dir:
        print("[WEAKEN_PATCH] ERROR: framework-arduinoespressif32 package not found.")
        return

    lib_path = os.path.join(platform_dir, "tools", "sdk", "esp32", "lib", "libnet80211.a")
    bak_path  = lib_path + ".bak"

    # Locate objcopy from the toolchain
    toolchain_dir = env.PioPlatform().get_package_dir("toolchain-xtensa-esp32")
    if not toolchain_dir:
        print("[WEAKEN_PATCH] ERROR: toolchain-xtensa-esp32 not found.")
        return

    objcopy = os.path.join(toolchain_dir, "bin", "xtensa-esp32-elf-objcopy.exe")
    nm      = os.path.join(toolchain_dir, "bin", "xtensa-esp32-elf-nm.exe")

    if not os.path.isfile(lib_path):
        print(f"[WEAKEN_PATCH] ERROR: libnet80211.a not found at: {lib_path}")
        return

    # Log framework version
    pkg_json = os.path.join(platform_dir, "package.json")
    fw_version = "unknown"
    if os.path.isfile(pkg_json):
        import json
        with open(pkg_json) as f:
            try:
                fw_version = json.load(f).get("version", "unknown")
            except Exception:
                pass
    print(f"[WEAKEN_PATCH] Framework version: {fw_version}")

    # Step 1: Inspect current symbol visibility
    try:
        nm_result = subprocess.check_output(
            [nm, "--defined-only", "-g", lib_path],
            stderr=subprocess.STDOUT
        ).decode(errors="replace")
    except subprocess.CalledProcessError as e:
        print(f"[WEAKEN_PATCH] nm failed: {e}")
        return

    already_weak = False
    for line in nm_result.splitlines():
        if "ieee80211_raw_frame_sanity_check" in line:
            # 'W' = weak, 'T' = strong (text section)
            parts = line.split()
            if parts:
                sym_type = parts[-2] if len(parts) >= 2 else "?"
                print(f"[WEAKEN_PATCH] Symbol found: {line.strip()} (type='{sym_type}')")
                if sym_type.upper() == "W":
                    already_weak = True

    if already_weak:
        print("[WEAKEN_PATCH] Symbol is already WEAK — no patch needed.")
        # Ensure backup exists even if we skip patching
        if not os.path.isfile(bak_path):
            shutil.copy2(lib_path, bak_path)
            print(f"[WEAKEN_PATCH] Backup created (retroactive): {bak_path}")
        else:
            print(f"[WEAKEN_PATCH] Backup already exists: {bak_path}")
        return

    # Step 2: Backup BEFORE modifying
    if not os.path.isfile(bak_path):
        shutil.copy2(lib_path, bak_path)
        print(f"[WEAKEN_PATCH] Backup created: {bak_path}")
    else:
        print(f"[WEAKEN_PATCH] Backup already exists — skipping: {bak_path}")

    # Step 3: Run objcopy --weaken-symbol
    print("[WEAKEN_PATCH] Applying --weaken-symbol patch...")
    try:
        subprocess.check_call([
            objcopy,
            "--weaken-symbol=ieee80211_raw_frame_sanity_check",
            lib_path,
            lib_path
        ])
        print("[WEAKEN_PATCH] SUCCESS: ieee80211_raw_frame_sanity_check is now WEAK.")
    except subprocess.CalledProcessError as e:
        print(f"[WEAKEN_PATCH] ERROR: objcopy failed: {e}")
        return

    # Step 4: Verify the result
    try:
        verify = subprocess.check_output(
            [nm, "--defined-only", "-g", lib_path],
            stderr=subprocess.STDOUT
        ).decode(errors="replace")
        for line in verify.splitlines():
            if "ieee80211_raw_frame_sanity_check" in line:
                print(f"[WEAKEN_PATCH] Verified post-patch: {line.strip()}")
    except subprocess.CalledProcessError:
        print("[WEAKEN_PATCH] WARNING: Could not verify post-patch symbol state.")

# Register as a pre-build action
env.AddPreAction("buildprog", weaken_sanity_check)
