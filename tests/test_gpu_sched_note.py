"""Compile and run the start-up log line about Windows' hardware-accelerated GPU scheduling (gpu/vk/gpu_sched_note.h; no game/GPU)."""
import argparse
import subprocess
import tempfile
from pathlib import Path

RUN_ALL_ARGS = ["--cxx", "{cxx}"]

ROOT = Path(__file__).resolve().parents[1]


def check_source():
    """mkwvk_init still reads the setting and logs the line, on Windows only, without a link to advapi32"""
    src = (ROOT / "gpu/vk/vk_main.cpp").read_text(encoding="utf-8")
    for needle in ('#include "gpu_sched_note.h"', r'"SYSTEM\\CurrentControlSet\\Control\\GraphicsDrivers", "HwSchMode"',
                   'LoadLibraryExA("advapi32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32)',
                   "gpu_sched_note::text((int)mode, dev.props.vendorID, line, sizeof line);"):
        if needle not in src:
            raise SystemExit("vk_main.cpp no longer has: " + needle)
    init = src[src.index("const MkwVkRenderer* mkwvk_init(const MkwVkHost* h) {"):]
    at = init.index("gpu_sched_note::text(")
    if init.rfind("#ifdef _WIN32", 0, at) < 0 or init.rfind("#ifdef _WIN32", 0, at) < init.rfind("#endif", 0, at):
        raise SystemExit("the setting is no longer read under _WIN32 only")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cxx", default="g++")
    args = parser.parse_args()
    check_source()
    with tempfile.TemporaryDirectory(prefix="gpu-sched-note-") as tmp:
        exe = Path(tmp) / "gpu-sched-note"
        subprocess.run([args.cxx, "-std=c++20", "-Wall", "-Wextra", "-Werror",
                        str(ROOT / "tests/test_gpu_sched_note.cpp"), "-o", str(exe)], check=True)
        subprocess.run([str(exe)], check=True)


if __name__ == "__main__":
    main()
