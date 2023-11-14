#ifndef __PACKET_HPP__
#define __PACKET_HPP__
#include <string>
#include <netinet/in.h>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <utility>

#define BLOCKSIZE 1024 
// The maximum size of a request packet
#define RQ_PACKETSIZE 512
#define DEFAULT_BLOCKSIZE 512
#define ERROR_MSG_SIZE 512

typedef struct {
    char ip[INET_ADDRSTRLEN];
    int port;
}ip_t;

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
    OACK = 6,
    NONE = 7,
};

enum class Mode {
    NETASCII = 1,
    OCTET = 2,
    MAIL = 3,
    NONE = 4
};
void print_buffer(char *buffer, int len);

enum class OptName {
    BLKSIZE,
    TSIZE,
    TIMEOUT,
    NONE
};

typedef struct {
    std::string name;
    std::string value;
} option_t;

class Packet {
    ip_t src;
    public:
        int blockid;
        Opcode opcode;
        std::vector<option_t> options;
        int len = 0;
        static std::string convert_mode_to_str(Mode mode); 

};

class RQ_packet : public Packet {
    public:
        std::string filename;
        std::string mode;
        char buffer[RQ_PACKETSIZE];
        RQ_packet();
        ~RQ_packet();
        RQ_packet(char *buffer);
        RQ_packet(Opcode new_opcode, std::string new_filename, 
                Mode new_mode, std::vector<option_t> options);

};

class DATA_packet : public Packet {
    public:
        char data[BLOCKSIZE] = {0};
        char buffer[4 + BLOCKSIZE] = {0};
        int data_size;
        DATA_packet(int blockid, char *data, int data_size);
        DATA_packet(char *buffer, int packet_len);
        // DATA_packet(char *buffer);
};

class ACK_packet : public Packet {
    public:
        char buffer[4];
        ACK_packet(int block);
        ACK_packet(char *buffer);
};
class OACK_packet : public Packet {
    public:
        char buffer[RQ_PACKETSIZE];
        OACK_packet(std::vector<option_t> options);
        OACK_packet(char *buffer);
};

class ERROR_packet : public Packet {
    public:
        char buffer[ERROR_MSG_SIZE];
        Error error_code;
        std::string error_msg;
        ERROR_packet(Error error_code, std::string error_msg);
        ERROR_packet(char *buffer);
};




#endif // __PACKET_HPP__
