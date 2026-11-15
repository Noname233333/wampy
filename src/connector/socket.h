#ifndef WAMPY_SOCKET_H
#define WAMPY_SOCKET_H

#include <cerrno>
#include <chrono>
#include <cstring>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace Hagoromo {
    // One deadline covers connect, partial writes and the complete response.
    // Keep the client timeout and the plugin's Hide deadline on the same clock.
    constexpr int CommandTimeoutMs = 200;
    inline long long monotonicMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    struct SocketFd {
        int fd;
        explicit SocketFd(int value) : fd(value) {}
        ~SocketFd() {
            if (fd >= 0)
                close(fd);
        }
        SocketFd(const SocketFd &) = delete;
        SocketFd &operator=(const SocketFd &) = delete;
    };

    inline bool waitSocket(int fd, short events, long long deadline) {
        for (;;) {
            auto remaining = deadline - monotonicMs();
            if (remaining <= 0)
                return false;
            pollfd p{fd, events, 0};
            int ready = poll(&p, 1, static_cast<int>(remaining));
            if (ready < 0 && errno == EINTR)
                continue;
            return monotonicMs() < deadline && ready > 0 && !(p.revents & POLLNVAL) && (p.revents & (events | POLLERR | POLLHUP));
        }
    }

    // The peer closes its connection to delimit the protobuf reply.
    inline bool exchangeConnected(int fd, const char *data, size_t length, std::string *reply, long long deadline) {
        reply->clear();
        size_t sent = 0;
        while (sent < length) {
            if (!waitSocket(fd, POLLOUT, deadline))
                return false;
            auto count = send(fd, data + sent, length - sent, MSG_NOSIGNAL);
            if (count < 0 && (errno == EINTR || errno == EAGAIN))
                continue;
            if (count <= 0)
                return false;
            sent += count;
        }

        std::string response;
        while (waitSocket(fd, POLLIN, deadline)) {
            char buffer[4096];
            auto count = recv(fd, buffer, sizeof(buffer), 0);
            if (monotonicMs() >= deadline)
                return false;
            if (count < 0 && (errno == EINTR || errno == EAGAIN))
                continue;
            // Commands return small status messages; do not accept an unbounded peer.
            if (count < 0 || response.size() + count > 64 * 1024)
                return false;
            if (count == 0) {
                reply->swap(response);
                return !reply->empty();
            }
            response.append(buffer, count);
        }
        return false;
    }

    inline bool exchange(const char *path, const char *data, size_t length, std::string *reply, int timeout) {
        reply->clear();
        if (!path || strlen(path) >= sizeof(sockaddr_un::sun_path) || timeout <= 0)
            return false;
        SocketFd socket(::socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0));
        if (socket.fd < 0)
            return false;
        auto deadline = monotonicMs() + timeout;
        sockaddr_un address{};
        address.sun_family = AF_UNIX;
        strcpy(address.sun_path, path);
        if (connect(socket.fd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0) {
            // EAGAIN on a full Unix listen backlog is a failed connect, not a pending one.
            if (errno != EINPROGRESS || !waitSocket(socket.fd, POLLOUT, deadline))
                return false;
            int error = 0;
            socklen_t size = sizeof(error);
            if (getsockopt(socket.fd, SOL_SOCKET, SO_ERROR, &error, &size) || error)
                return false;
        }
        return exchangeConnected(socket.fd, data, length, reply, deadline);
    }
}

#endif
