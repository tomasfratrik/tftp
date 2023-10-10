#include "./packet.hpp"

std::string Packet::convert_mode_to_str(Mode mode) {
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

RQ_packet::RQ_packet(Opcode new_opcode, std::string new_filename, Mode new_mode) {
    opcode = new_opcode;
    filename = new_filename;
    mode = Packet::convert_mode_to_str(new_mode);
}



void RQ_packet::init_buffer() {
    *(uint16_t*)(&buffer[0]) = htons((int)opcode);
    len += 2;
    strcpy(&buffer[len], filename.c_str());
    len += filename.length();
    buffer[len] = '\0';
    len++;
    strcpy(&buffer[len], mode.c_str());
    len += mode.length();
    buffer[len] = '\0';
    len++;

}

void print_buffer(char *buffer, int len) {
    int opcode = ntohs(*(uint16_t*)(&buffer[0]));
    std::cout << opcode;
    for (int i = 2; i < len; i++) {
        if (buffer[i] == '\0') {
            std::cout << "0";
        } else{
            std::cout << buffer[i];
        }
    }
}