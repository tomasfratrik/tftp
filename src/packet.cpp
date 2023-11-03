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

RQ_packet::RQ_packet(Opcode new_opcode, std::string new_filename, 
                    Mode new_mode, std::vector<option_t> options) {
    opcode = new_opcode;
    filename = new_filename;
    mode = Packet::convert_mode_to_str(new_mode);

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
    for (auto opt : options) {
        strcpy(&buffer[len], opt.name.c_str());
        len += opt.name.length();
        buffer[len] = '\0';
        len++;
        strcpy(&buffer[len], opt.value.c_str());
        len += opt.value.length();
        buffer[len] = '\0';
        len++;
    }
}



// void RQ_packet::init_buffer() {

// }

ACK_packet::ACK_packet(Opcode opcode, int block) {
    *(uint16_t*)(&buffer[0]) = htons((int)opcode);
    *(uint16_t*)(&buffer[2]) = htons(block);
    this->len += 4;
}

void ACK_packet::print_buffer(char *buffer) {
    std::cout << "opcode: " << ntohs(*(uint16_t*)(&buffer[0])) << std::endl;
    std::cout << "block: " << ntohs(*(uint16_t*)(&buffer[2])) << std::endl;
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
    std::cout << std::endl;
}