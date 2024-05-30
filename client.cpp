#include <string.h>

#include "net.cpp"
#include "socket.cpp"
#include "utils.cpp"

int main(int argc, const char **argv) {
    if (argc < 2) {
        log("Usage: client <port>\n");
        return -1;
    }
    int port = atoi(argv[1]);
    log("Initializing sockets...\n");
    InitializeSockets();

    SocketHandle socket = SocketOpen(port);
    if (!socket) {
        log("Could not open a socket\n");
        return -1;
    }

    log("Using port %d\n", port);

    Address serverAddress = MakeAddress(127, 0, 0, 1, 30000);
    Connection conn = MakeConnection(socket, serverAddress);

    char buffer[100];
    for (;;) {
        if (!fgets(buffer, 100, stdin)) break;
        u32 sn = SendPacket(
            &conn, (Buffer){.bytes = (u8 *)buffer, .size = strlen(buffer) + 1});
        if (sn) {
            log("Packet sent: %du\n", sn);
        }
    }

    log("Shutting down...\n");
    ShutdownSockets();
}
