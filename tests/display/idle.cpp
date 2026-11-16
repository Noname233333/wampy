#include "../../src/backlight.h"
#include "../../src/render.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <thread>

int main() {
    std::string path = "/tmp/wampy-backlight-test-" + std::to_string(getpid());
    auto writeValue = [&](const char *value) {
        int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
        assert(fd >= 0);
        assert(write(fd, value, strlen(value)) == static_cast<ssize_t>(strlen(value)));
        close(fd);
    };
    writeValue("0\n");
    Backlight backlight(path.c_str());
    bool on = true;
    assert(backlight.sample(on) && !on);
    writeValue("255\n");
    assert(backlight.sample(on) && on);
    writeValue("invalid");
    assert(!backlight.sample(on));
    unlink(path.c_str());
    assert(!backlight.sample(on));
    for (int i = 0; i < 1000; i++) {
        RenderGate gate;
        std::atomic<bool> stop{false};
        std::atomic<int> passed{0};
        std::thread first([&]() {
            if (gate.waitUntilVisible(stop))
                ++passed;
        });
        std::thread second([&]() {
            if (gate.waitUntilVisible(stop))
                ++passed;
        });
        assert(passed == 0);
        gate = true;
        first.join();
        second.join();
        assert(passed == 2);
        gate = false;
        std::thread stopping([&]() { assert(!gate.waitUntilVisible(stop)); });
        stop = true;
        gate.notify();
        stopping.join();
    }
    std::cout << "PASS: real inotify brightness/errors and 1000 two-worker pause/wake/cancel rounds\n";
}
