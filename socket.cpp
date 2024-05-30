#ifndef socket_cpp
#define socket_cpp

#include <stdint.h>
#include <stdio.h>

#include "types.h"
#include "utils.cpp"

#define PLATFORM_WINDOWS 1
#define PLATFORM_MAC 2
#define PLATFORM_UNIX 3

#if defined(_WIN32)
#define PLATFORM PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define PLATFORM PLATFORM_MAC
#else
#define PLATFORM PLATFORM_UNIX
#endif

#if PLATFORM == PLATFORM_WINDOWS

#include <winsock2.h>
#pragma comment(lib, "wsock32.lib")

#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>

#endif

struct Address {
    union {
        u32 address;
        u8 digits[4];
    };
    u16 port;
};

struct CharArray {
    char chars[25];
};

Address MakeAddress(u8 a, u8 b, u8 c, u8 d, u16 port) {
    return (Address){.port = port,
                     .address = u32(a << 24 | b << 16 | c << 8 | d)};
}

CharArray AddressToString(Address a) {
    CharArray res;
    snprintf(res.chars, sizeof(res.chars), "%d.%d.%d.%d:%d", a.digits[3],
             a.digits[2], a.digits[1], a.digits[0], a.port);
    return res;
}

bool operator==(const Address& a1, const Address& a2) {
    return a1.address == a2.address && a1.port == a2.port;
}

bool operator!=(const Address& a1, const Address& a2) { return !(a1 == a2); }

typedef int SocketHandle;

SocketHandle SocketOpen(u16 port) {
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (s <= 0) {
        printf("failed to create socket\n");
        return 0;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(s, (const sockaddr*)&address, sizeof(sockaddr_in)) < 0) {
        printf("failed to bind socket\n");
        return 0;
    }

    // #if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

    //     int nonBlocking = 1;
    //     if (fcntl(s->handle, F_SETFL, O_NONBLOCK, nonBlocking) == -1) {
    //         printf("failed to set non-blocking\n");
    //         return false;
    //     }

    // #elif PLATFORM == PLATFORM_WINDOWS

    //     DWORD nonBlocking = 1;
    //     if (ioctlsocket(handle, FIONBIO, &nonBlocking) != 0) {
    //         printf("failed to set non-blocking\n");
    //         return false;
    //     }

    // #endif

    return s;
}

bool SocketSend(SocketHandle socket, Address dest, Buffer b) {
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(dest.address);
    addr.sin_port = htons(dest.port);

    int sent_bytes = sendto(socket, (const char*)b.bytes, b.size, 0,
                            (sockaddr*)&addr, sizeof(sockaddr_in));

    if (sent_bytes != b.size) {
        printf("failed to send packet\n");
        return false;
    }
    return true;
}

int SocketReceive(SocketHandle socket, Address* sender, Buffer* b) {
#if PLATFORM == PLATFORM_WINDOWS
    typedef int socklen_t;
#endif

    sockaddr_in from;
    socklen_t fromLength = sizeof(from);
    int byteCount = recvfrom(socket, (char*)b->bytes, b->size, 0,
                             (sockaddr*)&from, &fromLength);
    sender->address = ntohl(from.sin_addr.s_addr);
    sender->port = ntohs(from.sin_port);
    b->size = byteCount;
    return byteCount;
}

bool InitializeSockets() {
#if PLATFORM == PLATFORM_WINDOWS
    WSADATA WsaData;
    return WSAStartup(MAKEWORD(2, 2), &WsaData) == NO_ERROR;
#else
    return true;
#endif
}

void ShutdownSockets() {
#if PLATFORM == PLATFORM_WINDOWS
    WSACleanup();
#endif
}

#endif
