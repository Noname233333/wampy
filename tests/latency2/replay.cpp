// Exercise the production input callback, main loop / protobuf adapter and Qt
// controller methods. Test doubles represent device input, clock, Qt/Mali and the
// socket transport. Generated .inc files contain the unmodified production bodies.
#include "production.hpp"
#include "command.pb.h"
#include <atomic>
#include <cassert>
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#define DLOG(...) ((void)0)

static long long elapsed;
static int fakeClock(clockid_t, struct timespec *out) {
    out->tv_sec=elapsed/1000;
    out->tv_nsec=(elapsed%1000)*1000000;
    return 0;
}

namespace wampy_resume {
    static long long monotonicMs() { return elapsed; }
    struct ResumeTrace { explicit ResumeTrace(const char *) {} void record(const char *, std::uint64_t=0, int=0) {} };
    using RenderGate=std::atomic<bool>;
    extern bool lit;
    struct DisplayMonitor {
        explicit DisplayMonitor(const char *) {}
        bool sample(bool &on) { on=lit; return true; }
        int fd() const { return 0; }
    };
    bool lit=true;
}
static int access(const char *,int) { return -1; }
#define F_OK 0
static void traceHandoff(const char *, std::uint64_t=0, int=0) {}

using quint64=std::uint64_t;
namespace Qt { enum ConnectionType { DirectConnection, QueuedConnection }; }
struct QThread { static QThread *currentThread() { static QThread owner; return &owner; } };
struct QObject {
    static void destroyed() {}
    QThread *thread() { return QThread::currentThread(); }
    template<class W, class Signal, class C, class Handler, class... Rest>
    static void connect(W *w, Signal signal, C *, Handler handler, Rest...) { w->bind(signal,handler); }
};
struct QOpenGLContext {
    static bool current;
    static QOpenGLContext *currentContext() { static QOpenGLContext ctx; return current ? &ctx : nullptr; }
    QOpenGLContext *functions() { return this; }
    void glFinish() {}
};
bool QOpenGLContext::current=false;
struct QQuickWindow : QObject {
    bool visible=true, exposed=true, keepVisible=false;
    long readyAt=0, hideCost=0;
    std::function<void(bool)> changed;
    std::function<void()> stopped;
    static void visibleChanged(bool) {}
    static void sceneGraphAboutToStop() {}
    void bind(void (*signal)(bool),const std::function<void(bool)> &handler) {
        if(signal==&visibleChanged) changed=handler;
    }
    void bind(void (*signal)(),const std::function<void()> &handler) {
        if(signal==&sceneGraphAboutToStop) stopped=handler;
    }
    QOpenGLContext *openglContext() { return QOpenGLContext::currentContext(); }
    bool isVisible() { return visible; }
    bool isExposed() { return exposed; }
    void show() { visible=exposed=true; if(changed) changed(true); }
    void hide() {
        elapsed+=hideCost;
        if(keepVisible||!visible) return;
        visible=exposed=false;
        if(changed) changed(false);
        QOpenGLContext::current=elapsed>=readyAt;
        if(stopped) stopped();
        QOpenGLContext::current=false;
    }
};
template<class T> T qobject_cast(QQuickWindow *window) { return window; }
struct QMetaObject {
    static bool invokeMethod(QObject *,const char *,Qt::ConnectionType) { return true; }
};
struct Controller {
    QQuickWindow *window, *handoffWindow=nullptr;
    QObject framework;
    QObject *FrameWork=&framework;
    std::atomic<bool> gpuDrained{false};
    quint64 handoffGeneration=0;
    struct { bool usbMounted=false; } provider;
    explicit Controller(QQuickWindow *w): window(w) {}
    void goToPlayer() {}
    void ObserveHandoff(Command::Command *);
    void Hide(Command::Command *);
    void Show(Command::Command *);
    void GetWindowStatus(Command::Command *);
};
#define clock_gettime fakeClock
#include "qt.inc"
#undef clock_gettime

enum { GLFW_KEY_HOLD=1, GLFW_KEY_POWER, GLFW_KEY_PLAY, GLFW_KEY_PREV, GLFW_KEY_NEXT,
       GLFW_KEY_VOLUMEDOWN, GLFW_KEY_VOLUMEUP };
struct GLFWwindow {};
static GLFWwindow glfw;
static GLFWwindow *window=&glfw;
static bool graphicsFailure=false;
static GLFWwindow *glfwGetCurrentContext() { return window; }
static void glfwMakeContextCurrent(GLFWwindow *) {}
static const int GL_VERSION=0;
static const char *glGetString(int) { return "test"; }
static void glFinish() {}
static double glfwGetTime() { return elapsed/1000.0; }
namespace ImGui {
    struct IO { void ClearInputKeys() {} void ClearInputMouse() {} };
    IO &GetIO() { static IO io; return io; }
}
static void ImGui_ImplGlfw_KeyCallback(GLFWwindow *,int,int,int,int) {}
#include "input.inc"

struct Scenario {
    struct Event { long at; int key; int action; };
    std::vector<Event> events;
    QQuickWindow native;
    Controller controller{&native};
    std::size_t index=0;
    long until=0, firstFrame=-1, wakeFrame=-1;
    int frames=0, hides=0, shows=0, entries=0, exits=0, inputEvents=0, audio=0;
    bool injectLate=false, timeout=false;
    std::vector<long> hideTimes;
    quint64 lastHide=0;
};
static Scenario *test;
namespace Hagoromo {
    struct HagoromoConnector {
        std::atomic<bool> storagePresent{true};
        bool power=false;
        void Play() { ++test->audio; }
        void PrevTrack() {}
        void NextTrack() {}
        void SetVolume(int,bool) {}
        static bool sendCMD(Command::Command *c) {
            switch(c->type()) {
            case Command::CMD_HIDE_WINDOW:
                ++test->hides;
                test->hideTimes.push_back(elapsed);
                test->lastHide=c->windowstatus().handoff_generation();
                test->controller.Hide(c);
                if(test->timeout) return false;
                break;
            case Command::CMD_GET_WINDOW_STATUS:
                test->controller.GetWindowStatus(c);
                if(test->injectLate && test->lastHide)
                    c->mutable_windowstatus()->set_handoff_generation(test->lastHide-1);
                break;
            case Command::CMD_SHOW_WINDOW:
                ++test->shows;
                test->controller.Show(c);
                break;
            default: assert(false);
            }
            return true;
        }
    };
}
static bool glfwWindowShouldClose(GLFWwindow *) { return ++elapsed>=test->until; }
static void glfwPollEvents() {
    while(test->index<test->events.size() && elapsed>=test->events[test->index].at) {
        auto e=test->events[test->index++];
        ++test->inputEvents;
        if(e.key==GLFW_KEY_POWER) {
            wampy_resume::lit=!wampy_resume::lit;
            if(wampy_resume::lit) test->native.show();
        }
        key_callback(window,e.key,0,e.action,0);
    }
}
static bool wampyInputWasReset() { return false; }
static void wampyWaitInput(int,int timeout) {
    auto next=test->index<test->events.size()?test->events[test->index].at:test->until;
    elapsed=timeout<0?next-1:std::min<long long>(elapsed+timeout,next-1);
}
static void replay(Scenario &scenario) {
    elapsed=0;
    test=&scenario;
    wampy_resume::lit=true;
    render=false;
    hold_toggled=power_pressed=false;
    hold_value=ACTION_RELEASE;
    sleepingKeys.clear();
    Hagoromo::HagoromoConnector device;
    auto connector=&device;
    struct { bool disableKeysWhenPowerOff=false; struct { bool swapTrackButtons=false; } misc; } config;
    struct {
        void ApplyUserSelection(bool enter) { if(enter) ++test->entries; else ++test->exits; }
        void KeyHandler() {}
    } skin;
#include "loop.inc"
        assert(!test->native.visible && !test->native.exposed && test->controller.gpuDrained);
        ++test->frames;
        if(test->firstFrame<0) test->firstFrame=elapsed;
        if(elapsed>1000 && test->wakeFrame<0) test->wakeFrame=elapsed;
    }
}

int main(int argc,char **) {
    const bool broken=argc==2;
    Scenario initial;
    initial.events={{20,GLFW_KEY_HOLD,1}};
    initial.native.readyAt=80;
    initial.until=2300;
    replay(initial);
    assert(initial.inputEvents==1 && initial.hides>=1);
    assert(broken ? initial.frames==0 && initial.shows>0 :
                   initial.firstFrame>0 && initial.entries==1 && initial.shows==0);
    std::cout<<"first HOLD with simulated delayed GPU readiness: first hide "<<initial.hideTimes.front()
             <<" ms, first frame "<<initial.firstFrame<<" ms, shows "<<initial.shows<<"\n";
    if(broken) return 0;

    Scenario switching;
    switching.events={{20,GLFW_KEY_HOLD,1},{450,GLFW_KEY_HOLD,0},
                      {500,GLFW_KEY_HOLD,1},{950,GLFW_KEY_HOLD,0}};
    switching.until=1200;
    replay(switching);
    assert(switching.entries==2 && switching.exits==2 && switching.shows==2 && switching.native.visible);
    std::cout<<"PASS: first HOLD, exit, consecutive HOLD cycles\n";

    for(long off:{10000L,60000L}) {
        Scenario wake;
        wake.events={{20,GLFW_KEY_HOLD,1},{500,GLFW_KEY_POWER,0},
                     {600,GLFW_KEY_PLAY,0},{500+off,GLFW_KEY_POWER,0}};
        wake.until=1000+off;
        replay(wake);
        assert(wake.entries==1 && wake.audio==1 && wake.wakeFrame>500+off);
        std::cout<<"PASS: retained HOLD intent after "<<off<<" ms blackout, audio input during blackout\n";
    }
    for(int i=0;i<4;++i) {
        Scenario fail;
        fail.events={{20,GLFW_KEY_HOLD,1}};
        fail.until=3300;
        if(i==0) fail.native.keepVisible=true;
        if(i==1) fail.native.hideCost=205;
        if(i==2) fail.injectLate=true;
        if(i==3) { fail.timeout=true; fail.native.keepVisible=true; }
        replay(fail);
        assert(fail.inputEvents==1 && fail.entries==0 && fail.frames==0 && fail.shows>=1 && fail.native.visible);
    }
    std::cout<<"PASS: visible native, slow Hide deadline, stale confirmation and transport failure never draw and recover Sony\n";
}
