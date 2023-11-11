#ifndef __SERVER_HPP__
#define __SERVER_HPP__
#include <string>
#include "./args-server.hpp"
#include "./logger.hpp"
#include "./tftp.hpp"

class Server {
    struct sockaddr_in server, client;

    int port = DEFAULT_PORT;
    std::string root_dirpath; 
    Logger logger;
    int blocksize = DEFAULT_BLOCKSIZE;
    socklen_t len;

    int sock;
    public:
        Server(Args_server  *args);
        void WRQ();
        void run();
};

#endif // __SERVER_HPP__