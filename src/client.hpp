#ifndef __CLIENT_HPP__
#define __CLIENT_HPP__

#include <string>
#include <netinet/in.h>
#include "./packet.hpp"
#include "./args-client.hpp"
#include "./logger.hpp"
#include "./tftp.hpp"


class Client {
    struct hostent* servent;
    struct sockaddr_in server;
    std::string server_filepath;
    std::string dest_path;
    std::string hostname;
    int port = DEFAULT_PORT;
    bool port_changed = false;

    Opcode opcode;
    Mode mode = Mode::OCTET;
    Logger logger;
    socklen_t len;

    //timeout configuration
    struct timeval timeout;
    int curr_timeout;
    int default_timeout = 1;

    /// @brief options for RRQ/WRQ
    std::vector<option_t> options;
    ip_t src;
    ip_t dest;
    bool opt_mode = true;
    int blockid = 0;
    int blocksize = 0;
    int sock;

    public:
        Client(Args *args);

        // run client
        void run();
        void print_status();
        void validate_options(OACK_packet oack_packet);
        void react_to_first_response(char *buffer);
        void send_error_packet(Error errcode, std::string errmsg);
        void netascii_wrq();

        /**
         * @brief start the client in Write mode
         * 
         */
        void WRQ();

        /**
         * @brief send packet to server
         * 
         * @param buffer 
         * @param len 
         * @return int 
         */
        int send(char *buffer, int len);

        /**
         * @brief recive packetfrom server
         *  also set with a timeout 
         * @param buffer recive buffer 
         * @param len buffer length 
         * @return int  
         */
        int recv(char *buffer, int len);

        /**
         * @brief send request packet to server
         * 
         */
        void send_rq_packet();
        /**
         * @brief sends an empty data packet to server
         * 
         */
        void send_empty_data_packet_recv_ack();
        /**
         * @brief Set the Timeout object
         * 
         * @param seconds set timeout in seconds
         */
        void setTimeout(int seconds);

        /**
         * @brief auto send and recive packet
         * 
         * @param send_buffer send this buffer
         * @param send_len lenght of send buffer
         * @param recv_buffer recive buffer
         * @param recv_len lenght of recive buffer
         * @return lenght of recived packet 
         */
        int send_and_recv(char *send_buffer, int send_len, 
                        char *recv_buffer, int recv_len);
};
    

#endif // ___CLIENT_HPP__