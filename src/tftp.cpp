#include<stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "./tftp.hpp"
#include "./packet.hpp"


ip_t Utils::find_src(struct sockaddr_in *addr) {
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr->sin_addr), client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(addr->sin_port);
    ip_t src;
    strcpy(src.ip, client_ip);
    src.port = client_port;
    return src;
}

Opcode Utils::get_opcode(char *buffer, int pos) {
    return (Opcode)ntohs(*(uint16_t*)(&buffer[pos]));
}

int Utils::get_2byte_num(char *buffer, int pos) {
    try {
        int opcode = ntohs(*(uint16_t*)(&buffer[pos]));
        return opcode;
    }
    catch(int err){
        return -1;
    }
}

void Utils::set_2byte_num(char *buffer, int pos, int num) {
    *(uint16_t*)(&buffer[pos]) = htons(num);
}

std::string Utils::convert_mode_to_str(Mode mode) {
    switch(mode) {
        case Mode::NETASCII:
            return "netascii";
        case Mode::OCTET:
            return "octet";
        case Mode::MAIL:
            return "mail";
        default:
            return "";
    }
}

