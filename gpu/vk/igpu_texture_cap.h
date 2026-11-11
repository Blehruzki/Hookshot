// The textures' budget on an integrated GPU where memory is set aside for it (vk_texture.cpp). tests/test_igpu_texture_cap.py
#pragma once
#include <algorithm>
#include <cstdint>

namespace igpu_texture_cap {

// An integrated GPU's "VRAM" is the system's RAM, which the game's own ~10 GB also needs, so its textures get a budget by
// the RAM's size (tier: 3584 MB with 24 GB, vk_texture.cpp). Not where the firmware sets memory aside for the GPU alone (a
// handheld's "VRAM" setting; Windows reports it as the adapter's dedicated video memory, and takes it out of the RAM it
// reports): that memory is no use to the game otherwise, and the textures may have it less what else goes there (render
// targets, buffers, the desktop: 1536 MB) when that is more than the tier. On a ROG Xbox Ally X with 6 GB set aside,
// Kakariko Village holds 3.3-4.4 GB of textures; at 2560 MB the ones in use were evicted and came back empty, 21-24 wrong
// frames in 45 s of walking and turning against 0-2 at 4608 MB. dedicated: 0 when unknown
inline uint64_t bytes(uint64_t tier, uint64_t dedicated) {
    constexpr uint64_t others = 1536ull << 20;
    return std::max(tier, dedicated > others ? dedicated - others : 0);
}

} // namespace igpu_texture_cap
