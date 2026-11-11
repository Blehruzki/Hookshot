// The textures' budget on an integrated GPU (gpu/vk/igpu_texture_cap.h): the RAM tier's where the GPU shares the system's
// RAM, the memory set aside for it less 1536 MB where that is more. tests/test_igpu_texture_cap.py
#include "../gpu/vk/igpu_texture_cap.h"

#include <cstdio>
#include <cstdlib>

namespace {
int failures = 0;
void expect(uint64_t tier_mb, uint64_t dedicated_mb, uint64_t want_mb) {
    const uint64_t got = igpu_texture_cap::bytes(tier_mb << 20, dedicated_mb << 20) >> 20;
    if (got != want_mb)
        std::printf("FAIL: tier %llu MB, %llu MB dedicated: %llu MB, expected %llu MB\n", (unsigned long long)tier_mb,
                    (unsigned long long)dedicated_mb, (unsigned long long)got, (unsigned long long)want_mb), failures++;
}
} // namespace

int main() {
    expect(1536, 0, 1536);       // (unknown: Linux, or no adapter matched: the tier, never more)
    expect(2560, 512, 2560);     // (a 16 GB laptop's Radeon 780M: shared memory)
    expect(2560, 2048, 2560);
    expect(2560, 4096, 2560);    // (4 GB set aside: no more than the tier gives)
    expect(2560, 4097, 2561);
    expect(3584, 4096, 3584);
    expect(3584, 6144, 4608);    // (ROG Xbox Ally X: 24 GB with 6 GB set aside)
    expect(4096, 8192, 6656);
    expect(6144, 4096, 6144);    // (never less than the tier)
    if (failures) return EXIT_FAILURE;
    std::printf("PASS: the integrated GPU's texture budget follows the memory set aside for it\n");
    return EXIT_SUCCESS;
}
