// jobs_wait (gpu/vk/vk_jobs.cpp): a waited-for job still in the queue runs on the waiting thread at once, instead of after
// every job queued before it. Built against stand-ins for the renderer's headers (tests/test_jobs_wait_runs_queued.py).
#include "vk_jobs.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <thread>
#include <vector>

using namespace mkwvk;

namespace {

int failures = 0;
void expect(bool ok, const char* what) {
    if (!ok) std::printf("FAIL: %s\n", what), failures++;
}

// (every worker held in a job until released, so that a job pushed after them stays queued)
std::mutex gate_mu;
std::condition_variable gate_cv;
bool gate_open = false;
std::atomic<int> held{0};
void hold() {
    held++;
    std::unique_lock lk(gate_mu);
    gate_cv.wait(lk, [] { return gate_open; });
}
void release() {
    {
        std::lock_guard lk(gate_mu);
        gate_open = true;
    }
    gate_cv.notify_all();
}

} // namespace

int main() {
    const unsigned workers = 32;   // (more than start() makes: every worker is held, and some holds stay queued)
    std::vector<std::atomic<bool>> holds_done(workers);
    for (unsigned i = 0; i < workers; i++)
        jobs_push([&holds_done, i] { hold(); holds_done[i].store(true); jobs_notify(); }, false, &holds_done[i]);
    const auto t0 = std::chrono::steady_clock::now();
    while (held.load() < 2 && std::chrono::steady_clock::now() - t0 < std::chrono::seconds(5)) std::this_thread::yield();
    expect(held.load() >= 2, "the workers took the holding jobs");

    std::atomic<bool> ready{false};
    std::thread::id ran_on{};
    jobs_push([&] { ran_on = std::this_thread::get_id(); ready.store(true, std::memory_order_release); }, false, &ready);
    jobs_wait(ready);   // (every worker is held: before, this waited until release(), which nobody calls)
    expect(ready.load(), "the waited-for job ran");
    expect(ran_on == std::this_thread::get_id(), "it ran on the waiting thread");

    // (jobs_wait_for: every worker is still held, so the job stays queued; the wait gives up and leaves it to a worker)
    const unsigned hw = std::thread::hardware_concurrency();
    const int made = (int)(hw >= 16 ? 6u : std::clamp(hw / 3, 2u, 4u));   // (start()'s count: all of them held)
    while (held.load() < made && std::chrono::steady_clock::now() - t0 < std::chrono::seconds(5)) std::this_thread::yield();
    expect(held.load() >= made, "every worker is in a holding job");
    std::atomic<bool> slow{false};
    std::thread::id slow_on{};
    jobs_push([&] { slow_on = std::this_thread::get_id(); slow.store(true, std::memory_order_release); }, false, &slow);
    const auto w0 = std::chrono::steady_clock::now();
    expect(!jobs_wait_for(slow, 50), "a bounded wait gives up while every worker is busy");
    const auto waited = std::chrono::steady_clock::now() - w0;
    expect(waited >= std::chrono::milliseconds(50) && waited < std::chrono::seconds(5), "after about the time it was given");
    expect(!slow.load(), "and does not run the job on the waiting thread");

    std::atomic<bool> unflagged{false};
    jobs_push([&] { unflagged.store(true, std::memory_order_release); jobs_notify(); });   // (no flag: left in the queue)
    release();
    jobs_wait(unflagged);   // (taken by a worker once released)
    expect(unflagged.load(), "a job pushed without its flag still runs on a worker");
    expect(jobs_wait_for(slow, 10000), "the job a bounded wait gave up on is done by a worker later");
    expect(slow_on != std::this_thread::get_id(), "on a worker's thread");
    for (unsigned i = 0; i < workers; i++) jobs_wait(holds_done[i]);

    if (failures) return EXIT_FAILURE;
    std::printf("PASS: a waited-for job still queued runs on the waiting thread at once\n");
    std::fflush(stdout);
    std::_Exit(EXIT_SUCCESS);   // (the workers are detached and wait forever)
}
