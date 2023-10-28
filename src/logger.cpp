#include "./logger.hpp"
#include <cstring>

ip_t Logger::find_src(struct sockaddr_in *addr) {
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr->sin_addr), client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(addr->sin_port);
    ip_t src;
    strcpy(src.ip, client_ip);
    src.port = client_port;
    return src;
}

Opcode Logger::get_opcode(char *buffer, int pos) {
    return (Opcode)ntohs(*(uint16_t*)(&buffer[pos]));
}

void Logger::log(char *buffer, int packet_len,struct sockaddr_in *client_addr) {
    ip_t src = Logger::find_src(client_addr);
    int curr_len = 0;
    // each packet by protocol has opcode as 2 byte number
    Opcode opcode = Logger::get_opcode(buffer, curr_len);
    int filesize;

    switch(opcode) {
        case Opcode::RRQ:
            break;
        case Opcode::WRQ:
            std::cerr << "WRQ " << src.ip << ":" << src.port << " ";
            curr_len +=2;
            std::cerr << &buffer[curr_len]<< " ";
            filesize = strlen(&buffer[2]) + 1;
            curr_len += filesize;
            std::cerr << &buffer[curr_len];
            std::cerr << std::endl;
            break;
        default:
            error_exit("Invalid opcode");
            break;
    }


}