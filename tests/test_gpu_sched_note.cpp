// gpu_sched_note::text (gpu/vk/gpu_sched_note.h): the start-up log line about Windows' hardware-accelerated GPU
// scheduling, and the warning on an AMD GPU with it on. No game, no GPU.
#include "../gpu/vk/gpu_sched_note.h"

#include <cstdio>
#include <cstring>

namespace {
int failures = 0;
void expect(bool ok, const char* what) {
    if (!ok) std::printf("FAIL: %s\n", what), failures++;
}
}  // namespace

int main() {
    char line[512];
    // on, AMD: the warning, with where the setting is
    expect(gpu_sched_note::text(2, 0x1002, line, sizeof line), "AMD with it on warns");
    expect(std::strstr(line, "is ON (HwSchMode 2)") != nullptr, "the warning says it is on");
    expect(std::strstr(line, "Advanced graphics settings") != nullptr, "the warning says where the setting is");
    // on, another vendor: the state only
    expect(!gpu_sched_note::text(2, 0x10DE, line, sizeof line), "NVIDIA with it on does not warn");
    expect(std::strcmp(line, "vk: Windows' hardware-accelerated GPU scheduling is on (HwSchMode 2)") == 0, "on, as a plain line");
    // off
    expect(!gpu_sched_note::text(1, 0x1002, line, sizeof line), "AMD with it off does not warn");
    expect(std::strcmp(line, "vk: Windows' hardware-accelerated GPU scheduling is off (HwSchMode 1)") == 0, "off");
    // not set or not read
    expect(!gpu_sched_note::text(0, 0x1002, line, sizeof line), "not set does not warn");
    expect(std::strcmp(line, "vk: Windows' hardware-accelerated GPU scheduling is not set (HwSchMode 0)") == 0, "not set");
    // a short buffer is not overrun
    char small[16];
    gpu_sched_note::text(2, 0x1002, small, sizeof small);
    expect(std::strlen(small) == sizeof small - 1, "a short buffer is cut, with its end");
    if (failures) return 1;
    std::printf("gpu_sched_note: ok\n");
    return 0;
}
