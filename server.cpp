#include "net.cpp"
#include "socket.cpp"
#include "utils.cpp"

int main() {
    log("Initializing sockets...\n");
    InitializeSockets();

    SocketHandle socket = SocketOpen(30000);
    if (!socket) {
        log("Could not open a socket\n");
        return -1;
    }

    log("Waiting for connection...\n");
    u8 buffer[256];
    Buffer buf = {.bytes = buffer, .size = sizeof(buffer)};

    for (;;) {
        Connection *conn = ReceivePacket(socket, &buf);
        if (conn == NULL) {
            log("Error receiving packet\n");
            continue;
        }
        log("Received packet %d from %s: %s\n", PacketSN(buf),
            AddressToString(conn->RemoteAddress).chars, PacketPayload(buf));
    }

    log("Shutting down...\n");
    ShutdownSockets();
}
