// What the log says at start-up about Windows' hardware-accelerated GPU scheduling (vk_main.cpp, mkwvk_init).
//
// With it on, Windows hands the scheduling of GPU work to the driver. On a Radeon 890M (an ROG Xbox Ally X, Adrenalin
// 26.10.1, Windows 11 26200) the game lost the device at the first frame after a save loaded, every run: Windows logged
// a LiveKernelEvent 141 (engine timeout, amdkmdag.sys, "HWS:1") while the GPU had finished everything the replay had
// submitted and the replay sat in a memory allocation for ~4.5 s. With the setting off the same build and save played on,
// and left the Deku Tree, with no timeout. The setting is the registry's HwSchMode (2: on, 1: off), read at start-up,
// so a report's log says which it was; on an AMD GPU with it on the line also says where the setting is.
#pragma once

#include <cstdint>
#include <cstdio>

namespace gpu_sched_note {

constexpr uint32_t kVendorAmd = 0x1002;

/// The line for mode (HwSchMode: 2 on, 1 off, anything else: not set or not read) and the GPU's vendor id, into out;
/// returns whether it is the AMD warning
inline bool text(int mode, uint32_t vendor, char* out, size_t cap) {
    const bool on = mode == 2;
    const bool warn = on && vendor == kVendorAmd;
    if (warn)
        snprintf(out, cap,
                 "vk: Windows' hardware-accelerated GPU scheduling is ON (HwSchMode 2). On a Radeon 890M the device was lost "
                 "right after a save loaded with it on, and not with it off: if the game freezes or loses the device, turn "
                 "it off (Settings > System > Display > Graphics > Advanced graphics settings) and restart Windows");
    else
        snprintf(out, cap, "vk: Windows' hardware-accelerated GPU scheduling is %s (HwSchMode %d)",
                 on ? "on" : mode == 1 ? "off" : "not set", mode);
    return warn;
}

}  // namespace gpu_sched_note
