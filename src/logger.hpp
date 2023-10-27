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
        static Opcode get_opcode(char *buffer, int pos);
        static void log(char *buffer, int len, struct sockaddr_in *client_addr);
        static ip_t find_src(struct sockaddr_in *client_addr);
};

#endif // __LOGGER_HPP__