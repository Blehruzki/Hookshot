"""Compile and run the rule for an integrated GPU's texture budget (gpu/vk/igpu_texture_cap.h; no game/GPU).

python -I -B tests/test_igpu_texture_cap.py
"""
import argparse
import subprocess
import tempfile
from pathlib import Path

RUN_ALL_ARGS = ["--cxx", "{cxx}"]

ROOT = Path(__file__).resolve().parents[1]


def check_source():
    """The budget still passes through the rule, the adapter is still asked, and the switch back is still read"""
    tex = (ROOT / "gpu/vk/vk_texture.cpp").read_text(encoding="utf-8")
    for needle in ('#include "igpu_texture_cap.h"', "igpu_texture_cap::bytes(budget, aside)",
                   "aside = shared_only ? 0 : dedicated_video_memory()", 'env_on("RX_VK_IGPU_SHARED_CAP")'):
        if needle not in tex:
            raise SystemExit("vk_texture.cpp no longer has: " + needle)
    vma = (ROOT / "gpu/vk/vk_vma.cpp").read_text(encoding="utf-8")
    fn = vma[vma.index("uint64_t dedicated_video_memory()"):]
    fn = fn[:fn.index("\n}\n")]
    for needle in ("d.VendorId == dev.props.vendorID", "d.DeviceId == dev.props.deviceID", "DedicatedVideoMemory",
                   "LOAD_LIBRARY_SEARCH_SYSTEM32"):
        if needle not in fn:
            raise SystemExit("dedicated_video_memory no longer has: " + needle)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cxx", default="g++")
    args = parser.parse_args()
    check_source()
    with tempfile.TemporaryDirectory(prefix="igpu-cap-") as tmp:
        exe = Path(tmp) / "igpu-cap"
        subprocess.run([args.cxx, "-std=c++20", "-Wall", "-Wextra", "-Werror",
                        str(ROOT / "tests/test_igpu_texture_cap.cpp"), "-o", str(exe)], check=True)
        subprocess.run([str(exe)], check=True)


if __name__ == "__main__":
    main()
