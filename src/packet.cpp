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

RQ_packet::~RQ_packet() {
    // ...
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

ACK_packet::ACK_packet(int block) {
    Utils::set_2byte_num(this->buffer, 0, (int)Opcode::ACK);
    Utils::set_2byte_num(this->buffer, 2, block);
    this->len += 4;
}

ACK_packet::ACK_packet(char *buffer) {
    this->opcode = Utils::get_opcode(buffer, 0);
    this->blockid = Utils::get_2byte_num(buffer, 2);
    this->len += 4;
}


OACK_packet::OACK_packet(std::vector<option_t> options){
    this->opcode = Opcode::OACK;
    this->options = options;
    Utils::set_2byte_num(this->buffer, 0, (int)this->opcode);
    this->len += 2;
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

OACK_packet::OACK_packet(char *buffer){
    this->opcode = Utils::get_opcode(buffer, 0);
    this->len += 2;
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

DATA_packet::DATA_packet(int blockid, char *data, int data_size) {
    this->opcode = Opcode::DATA;
    this->blockid = blockid;
    this->data_size = data_size;
    Utils::set_2byte_num(this->buffer, 0, (int)this->opcode);
    Utils::set_2byte_num(this->buffer, 2, this->blockid);
    memcpy(&this->buffer[4], data, this->data_size);
    this->len += 4 + this->data_size;
}

DATA_packet::DATA_packet(char *buffer, int packet_len) {
    this->opcode = Utils::get_opcode(buffer, 0);
    this->blockid = Utils::get_2byte_num(buffer, 2);
    this->data_size = packet_len - 4;
    memcpy(this->data, &buffer[4], this->data_size);
    this->len += packet_len;
}

ERROR_packet::ERROR_packet(Error error_code, std::string error_msg) {
    this->opcode = Opcode::ERROR;
    this->error_code = error_code;
    Utils::set_2byte_num(this->buffer, 0, (int)this->opcode);
    Utils::set_2byte_num(this->buffer, 2, (int)this->error_code);
    if (error_msg.length() > ERROR_MSG_SIZE-5) {
        // ...
    }
    std::string errmsg = error_msg;
    Utils::convert_string_to_netascii(&errmsg);
    memcpy(&this->buffer[4], errmsg.c_str(), errmsg.length());
    this->len += 4 + errmsg.length();
    this->buffer[this->len] = '\0';
}

ERROR_packet::ERROR_packet(char *buffer) {
    this->opcode = Utils::get_opcode(buffer, 0);
    this->error_code = (Error)Utils::get_2byte_num(buffer, 2);
    std::string errmsg = std::string(&buffer[4]);
    Utils::convert_string_from_netascii(&errmsg);
    this->error_msg = errmsg;
    this->len += 4 + this->error_msg.length();
    this->buffer[this->len] = '\0';
}