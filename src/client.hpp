#ifndef __CLIENT_HPP__
#define __CLIENT_HPP__

#include <string>
#include <netinet/in.h>
#include "./packet.hpp"
#include "./args-client.hpp"
#include "./tftp.hpp"

class Client {
    struct hostent* servent;
    struct sockaddr_in server;
    std::string hostname;
    int port = DEFAULT_PORT;
    std::string server_filepath;
    std::string dest_path;

    Opcode opcode;
    Mode mode = Mode::OCTET;
    socklen_t len;
    // Options opt;
    std::vector<option_t> options;
    bool opt_mode = false;
    int blockid;
    int blocksize;
    int sock;

    public:
        Client(Args *args);
        void run();
        void print_status();
        void WRQ();
        int send(char *buffer, int len);
        int recv(char *buffer, int len);

        void send_rq_packet();
};
    

#endif // ___CLIENT_HPP__