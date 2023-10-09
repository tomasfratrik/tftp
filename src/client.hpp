#ifndef __CLIENT_HPP__
#define __CLIENT_HPP__

#include <string>
#include <netinet/in.h>
#include "./packet.hpp"
#include "./args-client.hpp"

class Client {
    struct hostent* servent;
    struct sockaddr_in server;
    std::string hostname;
    int port = 1337;
    std::string server_filepath;
    std::string dest_path;

    Opcode opcode;
    Mode mode;
    int blockid;
    int blocksize;
    int sock;

    public:
        Client(Args *args);
        void run();
        void print_status();
        void WRQ();
};
    

#endif // ___CLIENT_HPP_