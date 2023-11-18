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
        int blocksize = SERVER_BLOCKSIZE;
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
        ssize_t filesize;
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
        void netascii_rrq(Config *cfg);
        std::string read_file_into_string(const std::string& filename);
        void send_empty_data_packet_recv_ack(Config *cfg);
        void send_error_packet(Error errcode, std::string errmsg, Config *cfg);
        bool file_exists(const std::string& filename);
        void change_option_if_exists(std::vector<option_t> *options, 
                            std::string name, std::string value);

        /**
         * @brief Read mode for client 
         * 
         * @param cfg current client session
         */
        void RRQ(Config *cfg);

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
         * @brief make adequate response to RRQ request
         * 
         * @param cfg current client session
         */
        void respond_to_rrq_rq(Config *cfg);

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
        void change_from_netascii(Config *cfg);
};


#endif // __SERVER_HPP__