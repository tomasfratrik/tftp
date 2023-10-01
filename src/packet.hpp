#ifndef __PACKET_HPP__
#define __PACKET_HPP__


#define BLOCKSIZE 512
// The maximum size of a request packet
#define PACKETSIZE 512

enum class Error {
    NOT_DEFINED = 0,
    FILE_NOT_FOUND = 1,
    ACCESS_VIOLATION = 2,
    DISK_FULL = 3,
    ILLEGAL_OPERATION = 4,
    UNKNOWN_TID = 5,
    FILE_ALREADY_EXISTS = 6,
    NO_SUCH_USER = 7,
    NONE = 8
};

enum class Opcode {
    RRQ = 1,
    WRQ = 2,
    DATA = 3,
    ACK = 4,
    ERROR = 5,
    NONE = 6
};

enum class Mode {
    NETASCII = 1,
    OCTET = 2,
    MAIL = 3,
    NONE = 4
};

#endif // __PACKET_HPP__