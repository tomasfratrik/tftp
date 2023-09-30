#ifndef __Client_HPP__
#define __Client_HPP__

#include <string>
#include <netinet/in.h>
#include "./packet.hpp"
#include "./args-client.hpp"

class Client {
    struct sockaddr_in addr;
    std::string hostname;
    int port = -1;
    std::string filepath;
    std::string dest_path;

    Opcode opcode;
    Mode mode;
    int blocksize;
    int sock;

    public:
        Client(Args *args);
        void run();
        void print_status();
};
    

#endif // __Client_HPP__