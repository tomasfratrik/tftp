#include "./packet.hpp"
#include "./tftp.hpp"

RQ_packet::RQ_packet() {
    filename = "";
    mode = "";
    len = 0;
}

RQ_packet::RQ_packet(char *buffer){
    this->opcode = Utils::get_opcode(buffer, 0);
    this->len += 2;
    this->filename = std::string(&buffer[this->len]);
    this->len += this->filename.length() + 1;
    this->mode = std::string(&buffer[this->len]);
    this->len += this->mode.length() + 1;

    if (len < RQ_PACKETSIZE) {
        std::string opt_name;
        std::string opt_value;
        while (this->len < RQ_PACKETSIZE) {
            opt_name = std::string(&buffer[this->len]);
            if (opt_name.empty()) {
                break;
            }
            this->len += opt_name.length() + 1;
            opt_value = std::string(&buffer[this->len]);
            this->len += opt_value.length() + 1;
            option_t opt = {.name = opt_name, .value = opt_value};
            this->options.push_back(opt);
        }
    }
}


RQ_packet::RQ_packet(Opcode new_opcode, std::string new_filename, 
                    Mode new_mode, std::vector<option_t> options) {
    this->opcode = new_opcode;
    this->filename = new_filename;
    this->mode = Utils::convert_mode_to_str(new_mode);
    this->options = options;

    Utils::set_2byte_num(this->buffer, 0, (int)this->opcode);
    this->len += 2;
    strcpy(&this->buffer[this->len], this->filename.c_str());
    this->len += this->filename.length();
    this->buffer[this->len] = '\0';
    this->len++;
    strcpy(&this->buffer[this->len], this->mode.c_str());
    this->len += this->mode.length();
    this->buffer[this->len] = '\0';
    this->len++;
    for (auto opt : this->options) {
        strcpy(&buffer[this->len], opt.name.c_str());
        this->len += opt.name.length();
        this->buffer[this->len] = '\0';
        this->len++;
        strcpy(&this->buffer[this->len], opt.value.c_str());
        this->len += opt.value.length();
        this->buffer[this->len] = '\0';
        this->len++;
    }
}

ACK_packet::ACK_packet(Opcode opcode, int block) {
    Utils::set_2byte_num(this->buffer, 0, (int)opcode);
    Utils::set_2byte_num(this->buffer, 2, block);
    this->len += 4;
}

ACK_packet::ACK_packet(char *buffer) {
    this->opcode = Utils::get_opcode(buffer, 0);
    this->blockid = Utils::get_2byte_num(buffer, 2);
    this->len += 4;
}
