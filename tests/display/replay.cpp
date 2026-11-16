// Exercise the production input callback, main loop / protobuf adapter and Qt
// controller methods. Test doubles represent device input, clock, Qt/Mali and the
// socket transport. Generated .inc files contain the unmodified production bodies.
#include "../../src/render.h"
#include "command.pb.h"
#include <atomic>
#include <cassert>
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#define DLOG(...) ((void)0)

static long long elapsed;
static int fakeClock(clockid_t, struct timespec *out) {
    out->tv_sec = elapsed / 1000;
    out->tv_nsec = (elapsed % 1000) * 1000000;
    return 0;
}

static bool lit = true, brightnessKnown = true;
struct Backlight {
    explicit Backlight(const char *) {}
    bool sample(bool &on) {
        on = lit;
        return brightnessKnown;
    }
    int fd() const { return 0; }
};
extern "C" int __wrap_nanosleep(const timespec *t, timespec *) {
    elapsed += t->tv_sec * 1000 + t->tv_nsec / 1000000;
    return 0;
}

using quint64 = std::uint64_t;
namespace Qt {
    enum ConnectionType { DirectConnection, QueuedConnection };
}
struct QThread {
    static QThread *currentThread() {
        static QThread owner;
        return &owner;
    }
};
struct QObject {
    static void destroyed() {}
    QThread *thread() { return QThread::currentThread(); }
    template <class W, class Signal, class C, class Handler, class... Rest> static void connect(W *w, Signal signal, C *, Handler handler, Rest...) {
        w->bind(signal, handler);
    }
};
struct QOpenGLContext {
    static bool current;
    static QOpenGLContext *currentContext() {
        static QOpenGLContext ctx;
        return current ? &ctx : nullptr;
    }
    QOpenGLContext *functions() { return this; }
    void glFinish() {}
};
bool QOpenGLContext::current = false;
struct QQuickWindow : QObject {
    bool visible = true, exposed = true, keepVisible = false;
    long readyAt = 0, hideCost = 0;
    std::function<void(bool)> changed;
    std::function<void()> stopped;
    static void visibleChanged(bool) {}
    static void sceneGraphAboutToStop() {}
    void bind(void (*signal)(bool), const std::function<void(bool)> &handler) {
        if (signal == &visibleChanged)
            changed = handler;
    }
    void bind(void (*signal)(), const std::function<void()> &handler) {
        if (signal == &sceneGraphAboutToStop)
            stopped = handler;
    }
    QOpenGLContext *openglContext() { return QOpenGLContext::currentContext(); }
    bool isVisible() { return visible; }
    bool isExposed() { return exposed; }
    void show() {
        visible = exposed = true;
        if (changed)
            changed(true);
    }
    void hide() {
        elapsed += hideCost;
        if (keepVisible || !visible)
            return;
        visible = exposed = false;
        if (changed)
            changed(false);
        QOpenGLContext::current = elapsed >= readyAt;
        if (stopped)
            stopped();
        QOpenGLContext::current = false;
    }
};
template <class T> T qobject_cast(QQuickWindow *window) { return window; }
struct QMetaObject {
    static bool invokeMethod(QObject *, const char *, Qt::ConnectionType) { return true; }
};
struct Controller {
    QQuickWindow *window, *handoffWindow = nullptr;
    QObject framework;
    QObject *FrameWork = &framework;
    std::atomic<bool> gpuDrained{false};
    quint64 handoffGeneration = 0;
    struct {
        bool usbMounted = false;
    } provider;
    explicit Controller(QQuickWindow *w) : window(w) {}
    void goToPlayer() {}
    void ObserveHandoff(Command::Command *);
    void Hide(Command::Command *);
    void Show(Command::Command *);
    void GetWindowStatus(Command::Command *);
};
#define clock_gettime fakeClock
#include "qt.inc"
#undef clock_gettime

enum { GLFW_KEY_HOLD = 1, GLFW_KEY_POWER, GLFW_KEY_PLAY, GLFW_KEY_PREV, GLFW_KEY_NEXT, GLFW_KEY_VOLUMEDOWN, GLFW_KEY_VOLUMEUP, GLFW_REPEAT = 2 };
struct GLFWwindow {};
static GLFWwindow glfw;
static GLFWwindow *window = &glfw;
static bool graphicsFailure = false;
static GLFWwindow *glfwGetCurrentContext() { return window; }
static void glfwMakeContextCurrent(GLFWwindow *) {}
static const int GL_VERSION = 0;
static const char *glGetString(int) { return "test"; }
static void glFinish() {}
static double glfwGetTime() { return elapsed / 1000.0; }
namespace ImGui {
    struct IO {
        void ClearInputKeys() {}
        void ClearInputMouse() {}
    };
    IO &GetIO() {
        static IO io;
        return io;
    }
}
static void ImGui_ImplGlfw_KeyCallback(GLFWwindow *, int, int, int, int) {}
#include "input.inc"

struct Scenario {
    struct Event {
        long at;
        int key;
        int action;
    };
    std::vector<Event> events;
    QQuickWindow native;
    Controller controller{&native};
    std::size_t index = 0;
    long until = 0, firstFrame = -1, wakeFrame = -1;
    long wakeAt = 0;
    int frames = 0, hides = 0, shows = 0, entries = 0, exits = 0, inputEvents = 0, audio = 0;
    bool injectLate = false, timeout = false, graphicsError = false, legacyPlugin = false, nativeTakeover = false;
    std::vector<long> hideTimes;
    quint64 lastHide = 0;
};
static Scenario *test;
enum HgrmToggleAction { Show, Hide };
namespace Hagoromo {
    constexpr int CommandTimeoutMs = 200;
    static long long monotonicMs() { return elapsed; }
    struct HagoromoConnector {
        uint64_t handoffGeneration = 0;
        std::atomic<bool> storagePresent{true};
        const char *brightnessPath = "unused";
        bool power = false;
        void Play() { ++test->audio; }
        void PrevTrack() {}
        void NextTrack() {}
        void SetVolume(int, bool) {}
        bool WindowHidden();
        void ToggleHgrm(HgrmToggleAction, RenderGate *);
        static bool sendCMD(Command::Command *c) {
            switch (c->type()) {
            case Command::CMD_HIDE_WINDOW:
                ++test->hides;
                test->hideTimes.push_back(elapsed);
                test->lastHide = c->windowstatus().handoff_generation();
                test->controller.Hide(c);
                if (test->timeout)
                    return false;
                break;
            case Command::CMD_GET_WINDOW_STATUS:
                if (test->nativeTakeover && test->frames > 0 && elapsed >= 500)
                    test->native.show();
                test->controller.GetWindowStatus(c);
                if (test->legacyPlugin)
                    c->mutable_windowstatus()->clear_handoff_protocol();
                if (test->injectLate && test->lastHide)
                    c->mutable_windowstatus()->set_handoff_generation(test->lastHide - 1);
                break;
            case Command::CMD_SHOW_WINDOW:
                ++test->shows;
                test->controller.Show(c);
                break;
            default:
                assert(false);
            }
            return true;
        }
    };
#include "handoff.inc"
}
static bool glfwWindowShouldClose(GLFWwindow *) { return ++elapsed >= test->until; }
static void glfwPollEvents() {
    while (test->index < test->events.size() && elapsed >= test->events[test->index].at) {
        auto e = test->events[test->index++];
        ++test->inputEvents;
        if (e.key == GLFW_KEY_POWER || e.key == 0) {
            lit = !lit;
            if (lit)
                test->native.show();
        }
        key_callback(window, e.key, 0, e.action, 0);
    }
    if (test->graphicsError && elapsed >= 500)
        graphicsFailure = true;
}
static bool wampyInputWasReset() { return false; }
static void wampyWaitInput(int) { elapsed = test->index < test->events.size() ? test->events[test->index].at - 1 : test->until; }
static void replay(Scenario &scenario) {
    elapsed = 0;
    test = &scenario;
    lit = true;
    brightnessKnown = true;
    render = false;
    hold_toggled = power_pressed = false;
    hold_value = ACTION_RELEASE;
    sleepingKeys.clear();
    Hagoromo::HagoromoConnector device;
    auto connector = &device;
    auto v = &device;
    struct {
        bool disableKeysWhenPowerOff = false;
        struct {
            bool swapTrackButtons = false;
        } misc;
    } config;
    struct {
        void ApplyUserSelection(bool enter) {
            if (enter)
                ++test->entries;
            else
                ++test->exits;
        }
        void KeyHandler() {}
        void LoadUpdatedSkin() { assert(!test->native.visible && test->controller.gpuDrained); }
    } skin;
#include "loop.inc"
    assert(!test->native.visible && !test->native.exposed && test->controller.gpuDrained);
    ++test->frames;
    if (test->firstFrame < 0)
        test->firstFrame = elapsed;
    if (test->wakeAt && elapsed > test->wakeAt && test->wakeFrame < 0)
        test->wakeFrame = elapsed;
}
}
int main() {
    Scenario initial;
    initial.events = {{1, GLFW_KEY_HOLD, ACTION_PRESS}, {500, GLFW_KEY_HOLD, ACTION_RELEASE}};
    initial.native.readyAt = 100;
    initial.until = 1000;
    replay(initial);
    assert(initial.frames > 0 && initial.entries == 1 && initial.exits == 1 && initial.native.visible);
    puts("PASS: first HOLD entry, fresh GPU proof, then HOLD exit");
    for (long offTime : {10000, 60000}) {
        Scenario wake;
        wake.wakeAt = 1000 + offTime;
        wake.events = {{1, GLFW_KEY_HOLD, ACTION_PRESS}, {1000, GLFW_KEY_POWER, ACTION_RELEASE}, {wake.wakeAt, GLFW_KEY_POWER, ACTION_RELEASE}};
        wake.until = wake.wakeAt + 1000;
        replay(wake);
        assert(wake.firstFrame > 0 && wake.wakeFrame > wake.wakeAt && wake.entries == 1 && wake.hides == 2);
    }
    puts("PASS: 10 s and 60 s POWER resume without reapplying entry features");
    Scenario asleepExit;
    asleepExit.events = {{1, GLFW_KEY_HOLD, ACTION_PRESS},
        {1000, GLFW_KEY_POWER, ACTION_RELEASE},
        {1100, GLFW_KEY_HOLD, ACTION_RELEASE},
        {1200, GLFW_KEY_POWER, ACTION_RELEASE}};
    asleepExit.until = 2000;
    replay(asleepExit);
    assert(asleepExit.hides == 1 && asleepExit.native.visible && asleepExit.exits == 1);
    puts("PASS: HOLD release while asleep cancels automatic return");
    Scenario repeat;
    for (int i = 0; i < 5; i++) {
        repeat.events.push_back({i * 1000 + 1, GLFW_KEY_HOLD, ACTION_PRESS});
        repeat.events.push_back({i * 1000 + 500, GLFW_KEY_HOLD, ACTION_RELEASE});
    }
    repeat.until = 5500;
    replay(repeat);
    assert(repeat.entries == 5 && repeat.exits == 5 && repeat.hides == 5);
    puts("PASS: repeated HOLD entry/exit");
    for (int failure = 0; failure < 7; failure++) {
        Scenario refused;
        refused.events = {{1, GLFW_KEY_HOLD, ACTION_PRESS}};
        refused.until = 5000;
        refused.timeout = failure == 0;
        refused.injectLate = failure == 1;
        refused.native.keepVisible = failure == 2;
        refused.native.hideCost = failure == 3 ? 201 : 0;
        refused.legacyPlugin = failure == 4;
        refused.native.readyAt = failure == 5 ? 10000 : 0;
        refused.controller.provider.usbMounted = failure == 6;
        replay(refused);
        assert(refused.frames == 0 && refused.native.visible && refused.hides <= 3 && refused.shows >= 1);
    }
    puts("PASS: lost/late Hide, stale generation, visible window, old plugin, absent GPU drain and USB export fail closed");
    Scenario takeover;
    takeover.events = {{1, GLFW_KEY_HOLD, ACTION_PRESS}};
    takeover.nativeTakeover = true;
    takeover.until = 1500;
    replay(takeover);
    assert(takeover.frames > 0 && takeover.native.visible && takeover.hides == 1);
    puts("PASS: observed native takeover yields without another Hide");
    Scenario broken;
    broken.events = {{1, GLFW_KEY_HOLD, ACTION_PRESS}};
    broken.graphicsError = true;
    broken.until = 1500;
    replay(broken);
    assert(broken.frames > 0 && broken.native.visible && broken.hides == 1);
    puts("PASS: graphics failure yields to Sony without another Hide");
    Scenario rapid;
    rapid.events = {{1, GLFW_KEY_HOLD, ACTION_PRESS}, {500, GLFW_KEY_POWER, ACTION_RELEASE}, {500, GLFW_KEY_POWER, ACTION_RELEASE}};
    rapid.until = 1500;
    replay(rapid);
    assert(rapid.frames > 0 && rapid.hides == 2 && !rapid.native.visible);
    puts("PASS: rapid POWER pair in one input batch invalidates old ownership");
    Scenario automatic;
    automatic.wakeAt = 5000;
    automatic.events = {{1, GLFW_KEY_HOLD, ACTION_PRESS}, {1000, 0, 0}, {5000, 0, 0}};
    automatic.until = 6000;
    replay(automatic);
    assert(automatic.hides == 2 && automatic.wakeFrame > 5000 && automatic.entries == 1);
    puts("PASS: automatic brightness off/on without a POWER event");
}
