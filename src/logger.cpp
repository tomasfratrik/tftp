#include "./logger.hpp"
#include <cstring>

ip_t Logger::find_src(struct sockaddr_in *client_addr) {
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr->sin_addr), client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(client_addr->sin_port);
    ip_t src;
    strcpy(src.ip, client_ip);
    src.port = client_port;
    return src;
}

Opcode Logger::get_opcode(char *buffer) {
    return (Opcode)ntohs(*(uint16_t*)(&buffer[0]));
}

void Logger::log(char *buffer, int len,struct sockaddr_in *client_addr) {
    ip_t src = Logger::find_src(client_addr);
    Opcode opcode = Logger::get_opcode(buffer);

    switch(opcode) {
        case Opcode::RRQ:
            break;
        case Opcode::WRQ:
            std::cout << "WRQ " << src.ip << ":" << src.port << " \"" << &buffer[2] << "\" " << &buffer[2 + strlen(&buffer[2]) + 1] << std::endl;
            break;
        default:
            error_exit("Invalid opcode");
            break;
    }


}