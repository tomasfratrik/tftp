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
        int port;
        bool opt_mode;
        ip_t src;
        ip_t dest;
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

        /**
         * @brief Write mode for client 
         * 
         * @param cfg current client session
         */
        void WRQ(Config *cfg);

        /**
         * @brief run server that handles incoming client requests
         * and threads them
         */
        void run();
        /**
         * @brief make adequate response to WRQ request
         * 
         * @param cfg current client session
         */
        void respond_to_wrq_rq(Config *cfg);
        /**
         * @brief receive payload from client
         * 
         * @param cfg current client session
         * @param dest where to send the packet
         * @param buffer payload to send
         * @param len  
         * @return int 
         */
        int send(Config *cfg, 
                        struct sockaddr_in *dest, char *buffer, int len);
        /**
         * @brief send payload to client
         * 
         * @param cfg current client session
         * @param dest where to send the packet
         * @param buffer buffer to send
         * @param buffer_len 
         * @return int 
         */
        int recv(Config *cfg, struct sockaddr_in *dest, char *buffer, int buffer_len);
        /**
         * @brief call this function to handle new incoming client
         *  
         * @param client_address 
         * @param recv_buffer payload from client 
         * @param bytes_read lenght of payload 
         */
        void handle_client(sockaddr_in client_address, char recv_buffer[RQ_PACKETSIZE], int bytes_read);
};


#endif // __SERVER_HPP__