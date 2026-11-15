#include "../../src/connector/socket.h"
#include <atomic>
#include <cassert>
#include <dirent.h>
#include <iostream>
#include <thread>
using namespace Hagoromo;
int fdCount() {
    int count = 0;
    auto d = opendir("/proc/self/fd");
    assert(d);
    while (readdir(d))
        ++count;
    closedir(d);
    return count;
}
int main() {
    int f[2];
    std::string reply;
    std::atomic<int> lateWrite{0};
    assert(socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, f) == 0);
    std::thread delayed([&]() {
        SocketFd peer(f[1]);
        char request;
        assert(waitSocket(peer.fd, POLLIN, monotonicMs() + 500));
        assert(read(peer.fd, &request, 1) == 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        lateWrite = send(peer.fd, "late", 4, MSG_NOSIGNAL);
    });
    {
        SocketFd client(f[0]);
        assert(!exchangeConnected(client.fd, "a", 1, &reply, monotonicMs() + 20) && reply.empty());
    }
    delayed.join();
    assert(lateWrite < 0);
    assert(socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, f) == 0);
    std::thread fresh([&]() {
        SocketFd peer(f[1]);
        char request;
        assert(waitSocket(peer.fd, POLLIN, monotonicMs() + 500));
        assert(read(peer.fd, &request, 1) == 1);
        assert(send(peer.fd, "fresh", 5, MSG_NOSIGNAL) == 5);
    });
    {
        SocketFd client(f[0]);
        assert(exchangeConnected(client.fd, "b", 1, &reply, monotonicMs() + 500) && reply == "fresh");
    }
    fresh.join();
    assert(socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, f) == 0);
    std::thread oversized([&]() {
        SocketFd peer(f[1]);
        char request;
        assert(waitSocket(peer.fd, POLLIN, monotonicMs() + 500));
        assert(read(peer.fd, &request, 1) == 1);
        std::string payload(70000, 'x');
        size_t sent = 0;
        auto deadline = monotonicMs() + 500;
        while (sent < payload.size() && waitSocket(peer.fd, POLLOUT, deadline)) {
            auto n = send(peer.fd, payload.data() + sent, payload.size() - sent, MSG_NOSIGNAL);
            if (n <= 0)
                break;
            sent += size_t(n);
        }
    });
    {
        SocketFd client(f[0]);
        assert(!exchangeConnected(client.fd, "x", 1, &reply, monotonicMs() + 500) && reply.empty());
    }
    oversized.join();

    // Pathname failures must also close the newly allocated socket.
    int missingBefore = fdCount();
    for (int i = 0; i < 200; ++i)
        assert(!exchange("/tmp/wampy-test-missing-socket", "x", 1, &reply, 10));
    assert(fdCount() == missingBefore);
    int before = fdCount();
    for (int i = 0; i < 200; ++i) {
        assert(socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, f) == 0);
        SocketFd a(f[0]), b(f[1]);
        assert(!exchangeConnected(a.fd, "x", 1, &reply, monotonicMs() + 1));
    }
    assert(fdCount() == before);
    std::cout << "PASS: real socketpair deadlines/late peer close/fresh RPC/oversized reply and allocated FD cleanup; missing pathname/FD cleanup covered; no "
                 "device endpoint\n";
}
