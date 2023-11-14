#ifndef __SERVER_HPP__
#define __SERVER_HPP__
#include <string>
#include "./args-server.hpp"
#include "./logger.hpp"
#include "./tftp.hpp"

#define SERVER_BLOCKSIZE 1024

class Config {
    public:
        char buffer[RQ_PACKETSIZE];
        struct sockaddr_in server, client;
        Logger logger;
        int blocksize = DEFAULT_BLOCKSIZE;
        int blockid = 0;
        struct timeval timeout;
        int curr_timeout;
        int len;
        int sock;
        bool opt_mode;
        std::string filename;
        std::string mode;
        std::vector<option_t> options;
        Config();
        void setTimeout(int seconds);
};

class Server {
    int port = DEFAULT_PORT;
    std::string root_dirpath; 
    public:
        Server(Args_server *args);
        void WRQ(Config *cfg);
        void run();
        void respond_to_wrq_rq(Config *cfg);
        int send(Config *cfg, 
                        struct sockaddr_in *dest, char *buffer, int len);
        // int recv(Config *cfg, 
        //         struct sockaddr_in *dest, char *buffer, int len);
        int recv(Config *cfg, struct sockaddr_in *dest, char *buffer, int buffer_len);
};


#endif // __SERVER_HPP__