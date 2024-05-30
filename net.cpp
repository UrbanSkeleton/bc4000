#ifndef net_cpp
#define net_cpp

#include <string.h>
#include <sys/socket.h>
#include <time.h>

#include "socket.cpp"
#include "utils.cpp"

const u32 protocolId =
    ((u32)'b') << 24 | ((u32)'c') << 16 | ((u32)'4') << 8 | ((u32)'0');

struct PacketMetadata {
    u32 SN;
    struct timespec Time;
};

struct Connection {
    SocketHandle Socket;
    Address RemoteAddress;
    PacketMetadata LastPacketReceived;
    PacketMetadata LastPacketSent;
    bool IsOpen;
};

Connection MakeConnection(SocketHandle socket, Address remoteAddress) {
    return (Connection){
        .Socket = socket, .RemoteAddress = remoteAddress, .IsOpen = true};
}

Buffer MakePacket(u32 sn, Buffer b) {
    static const int headerSize = 8;
    Buffer res = {.size = b.size + headerSize,
                  .bytes = (u8 *)malloc(b.size + headerSize)};
    ((u32 *)res.bytes)[0] = protocolId;
    ((u32 *)res.bytes)[1] = sn;
    memcpy(&res.bytes[headerSize], b.bytes, b.size);
    return res;
}

u32 SendPacket(Connection *conn, Buffer buf) {
    u32 sn = conn->LastPacketSent.SN + 1;
    Buffer msg = MakePacket(sn, buf);
    if (!SocketSend(conn->Socket, conn->RemoteAddress, msg)) {
        return 0;
    }
    timespec_get(&conn->LastPacketSent.Time, TIME_UTC);
    conn->LastPacketSent.SN = sn;
    return sn;
}

inline bool isPacket(Buffer b) {
    return b.size > 4 && ((u32 *)b.bytes)[0] == protocolId;
}

inline u32 PacketSN(Buffer b) { return ((u32 *)b.bytes)[1]; }
inline char *PacketPayload(Buffer b) { return (char *)(b.bytes + 8); }

static const int maxConnectionCount = 8;
static int connectionCount = 0;
static Connection connections[maxConnectionCount];

Connection *ReceivePacket(SocketHandle socket, Buffer *buf) {
    Address sender;
    int bytesRead = SocketReceive(socket, &sender, buf);
    if (!bytesRead) return NULL;
    if (!isPacket(*buf)) return NULL;
    for (int i = 0; i < connectionCount; i++) {
        if (connections[i].RemoteAddress == sender) return &connections[i];
    }
    if (connectionCount == maxConnectionCount) {
        log("Too many connections!");
        return NULL;
    }
    PacketMetadata md = {.SN = PacketSN(*buf)};
    timespec_get(&md.Time, TIME_UTC);
    connections[connectionCount++] = (Connection){.RemoteAddress = sender,
                                                  .IsOpen = true,
                                                  .LastPacketReceived = md,
                                                  .Socket = socket};
    return &connections[connectionCount - 1];
}

// Connection AcceptConnection(SocketHandle socket, Address remoteAddress,
//                             u32 sn) {
//     int bytesRead = SocketReceive(socket, &sender, &buf);

//     Connection conn = {.RemoteAddress = remoteAddress,
//                        .IsOpen = true,
//                        .LastPacketReceived = (PacketMetadata){.SN = sn},
//                        .Socket = socket};
//     timespec_get(&conn.LastPacketReceived.Time, TIME_UTC);
//     return conn;
// }

#endif
