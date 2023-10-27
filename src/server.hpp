#ifndef __SERVER_HPP__
#define __SERVER_HPP__
#include <string>
#include "./args-server.hpp"
#include "./logger.hpp"
#include "./tftp.hpp"

class Server {
    int port = DEFAULT_PORT;
    std::string root_dirpath; 
    Logger logger;

    public:
        Server(Args_server  *args);
        void run();
};

#endif // __SERVER_HPP__