"""
PlatformIO pre-build script: patch_libnet80211.py
===================================================
Replaces the official libnet80211.a with a binary-patched version
that permanently bypasses the 0xC0 and 0xA0 sanity checks.
"""

import os
import shutil

Import("env")

def patch_sanity_check(source, target, env):
    platform_dir = env.PioPlatform().get_package_dir("framework-arduinoespressif32")
    if not platform_dir:
        print("[WEAKEN_PATCH] ERROR: framework-arduinoespressif32 package not found.")
        return

    lib_path = os.path.join(platform_dir, "tools", "sdk", "esp32", "lib", "libnet80211.a")
    bak_path = lib_path + ".bak"
    patch_file = os.path.join(env.get("PROJECT_DIR"), "src", "patched_libnet80211.a.bin")

    if not os.path.isfile(lib_path):
        print(f"[WEAKEN_PATCH] ERROR: target libnet80211.a not found at: {lib_path}")
        return

    if not os.path.isfile(patch_file):
        print(f"[WEAKEN_PATCH] ERROR: patch file not found at: {patch_file}")
        return

    # Backup the original if no backup exists
    if not os.path.isfile(bak_path):
        shutil.copy2(lib_path, bak_path)
        print(f"[WEAKEN_PATCH] Original library backed up to: {bak_path}")

    # Copy the pre-patched binary over
    # (We compare sizes as a basic heuristic to see if it's already patched)
    if os.path.getsize(lib_path) != os.path.getsize(patch_file):
        shutil.copy2(patch_file, lib_path)
        print("[WEAKEN_PATCH] SUCCESS: libnet80211.a has been replaced with the patched binary.")
    else:
        print("[WEAKEN_PATCH] Library appears to be already patched. Skipping.")

env.AddPreAction("buildprog", patch_sanity_check)
