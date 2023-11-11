#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include "./packet.hpp"
#include "./error_exit.hpp"


class Logger {
    public: 
        static void log_packet(RQ_packet *packet, ip_t src);
        static void log_packet(ACK_packet *packet, ip_t src);
        static void log_packet(OACK_packet *packet, ip_t src);
};

#endif // __LOGGER_HPP__