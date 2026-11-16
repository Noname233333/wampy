#ifndef WAMPY_RENDER_H
#define WAMPY_RENDER_H

// The UI owns visibility. Animation workers sleep while rendering is disabled.
#include <atomic>
#include <condition_variable>
#include <mutex>

class RenderGate {
    std::atomic<bool> visible;
    std::mutex mutex;
    std::condition_variable changed;

  public:
    explicit RenderGate(bool visible = false) : visible(visible) {}
    bool load() const { return visible.load(); }
    operator bool() const { return load(); }
    RenderGate &operator=(bool value) {
        if (visible.exchange(value) != value)
            notify();
        return *this;
    }
    void notify() {
        std::lock_guard<std::mutex> lock(mutex);
        changed.notify_all();
    }
    bool waitUntilVisible(const std::atomic<bool> &stop) {
        std::unique_lock<std::mutex> lock(mutex);
        changed.wait(lock, [&]() { return visible.load() || stop.load(); });
        return !stop.load();
    }
};

#endif
