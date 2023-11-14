#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstring>
#include "./client.hpp"
#include "./error_exit.hpp"



Client::Client(Args *args){
    hostname = args->hostname;
    port = args->port;
    server_filepath = args->filepath;
    dest_path = args->dest_path;
    if(server_filepath.empty()){
        opcode = Opcode::WRQ;
    } else {
        opcode = Opcode::RRQ;
    }
    mode = Mode::OCTET;
}

void Client::print_status() {
    std::cout << "hostname: " << hostname << std::endl;
    std::cout << "port: " << port << std::endl;
    std::cout << "filepath: " << server_filepath << std::endl;
    std::cout << "dest_path: " << dest_path << std::endl;
}

int Client::send(char *buffer, int len){
    int n = sendto(this->sock, buffer, len, 0, 
            (struct sockaddr *)&(this->server), sizeof(this->server));

    if (n < 0){
        error_exit("sendto error");
    }
    return n;
}

int Client::recv(char *buffer, int len){
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(this->sock, &read_fds);

    sockaddr_in inaddr;
    this->len = sizeof(inaddr);
    int sel = select(this->sock + 1, &read_fds, nullptr, nullptr, &this->timeout);
    if (sel < 0) {
        // differ from possible -1 from recvfrom
        return -2;
    }

    int n = recvfrom(this->sock, buffer, len, 0, 
            (struct sockaddr *) &inaddr, &(this->len));
    return n;
}

void Client::send_rq_packet() {
    option_t blksize_opt = {.name = "blksize", .value = "1024"};
    option_t timeout_opt = {.name = "timeout", .value = "1"};
    option_t test = {.name = "tsize", .value = "1"};
    this->options.push_back(blksize_opt);
    this->options.push_back(test);
    this->options.push_back(timeout_opt);

    RQ_packet rq_packet(this->opcode, this->dest_path, this->mode, this->options);
    int n = this->send(rq_packet.buffer, rq_packet.len);
}

void Client::send_empty_data_packet_recv_ack() {
    Logger logger;
    char buffer[RQ_PACKETSIZE] = {0};
    DATA_packet data_packet(++this->blockid, nullptr, 0);
    int n = this->send(data_packet.buffer, data_packet.len);
    n = this->recv(buffer, RQ_PACKETSIZE);
    ACK_packet ack_packet(buffer);
    // ip_t src = Utils::find_src(&(this->server));
    logger.log_packet(&ack_packet, this->src);
}

int Client::send_and_recv(char *send_buffer, int send_len, 
                        char *recv_buffer, int recv_len) {
    int n;
    this->setTimeout(this->default_timeout);
    for (int retry = 0; retry <= MAX_TIMEOUT_TRIES; retry++) {
        if (retry == MAX_TIMEOUT_TRIES) {
            error_exit("TIMED OUT");
        }
        this->send(send_buffer, send_len);
        n = this->recv(recv_buffer, recv_len);
        if (n >= 0) {
            break;
        }
        int newTimeout = this->curr_timeout * 2;
        this->setTimeout(newTimeout);
    }
    this->setTimeout(this->default_timeout);
    return n;
}

void Client::send_error_packet(Error errcode, std::string errmsg) {
    ERROR_packet error_packet(errcode, errmsg);
    this->send(error_packet.buffer, error_packet.len);
}


void Client::validate_options(OACK_packet oack_packet) {
    int value;
    for (auto opt : oack_packet.options) {
        if (opt.name == "blksize") {
            // if value is less then 8 throw error
            try {
                value = std::stoi(opt.value);
            }
            catch(int err) {
                this->send_error_packet(Error::ILLEGAL_OPERATION, 
                        "blksize must be a number");
            }
            if (value < 8 || value > 65464) {
                this->send_error_packet(Error::ILLEGAL_OPERATION, 
                        "blksize must be greater than 8 and less than 65464");
            }
            this->blocksize = value;
        }
        else if (opt.name == "timeout") {
            try {
                    value = std::stoi(opt.value);
            }
            catch(int err) {
                this->send_error_packet(Error::ILLEGAL_OPERATION, 
                        "timeout must be a number");
            }
            if (value < 1 || value > 255) {
                this->send_error_packet(Error::ILLEGAL_OPERATION, 
                        "timeout must be greater than 1 and less than 255");
            }
            this->default_timeout = value;
            this->setTimeout(this->default_timeout);
        }
        else if (opt.name == "tsize") {
            // ...
        }
    }
}
void Client::react_to_first_response(char *buffer) {
    Opcode op = Utils::get_opcode(buffer, 0);
    int value;

    if(this->opcode == Opcode::WRQ){
        switch(op) {
            case Opcode::ACK:
            {
                ACK_packet ack_packet(buffer);
                this->logger.log_packet(&ack_packet, src);
                this->blocksize = DEFAULT_BLOCKSIZE;
            }
                break;
            case Opcode::OACK:
            {
                OACK_packet oack_packet(buffer);
                logger.log_packet(&oack_packet, src);
                this->validate_options(oack_packet);
            }
                break;
            case Opcode::ERROR:
            {
                std::cout << "ERROR packet recived" << std::endl;
            }
                break;
            default:
                error_exit("CAN'T FIND SUITABLE OPCODE");
                break;
        }
    }
    else if (this->opcode == Opcode::RRQ) {
        // ...
    }
}

void Client::WRQ() {
    char buffer[RQ_PACKETSIZE] = {0};
    int n;
    Logger logger;

    //
    // Initialize Write Request packet, and send it
    //
    option_t blksize_opt = {.name = "blksize", .value = "1024"};
    option_t timeout_opt = {.name = "timeout", .value = "1"};
    option_t tsize_opt = {.name = "tsize", .value = "1"};
    this->options.push_back(blksize_opt);
    this->options.push_back(timeout_opt);
    // this->options.push_back(tsize_opt);
    RQ_packet rq_packet(this->opcode, this->dest_path, this->mode, this->options);
    n = this->send_and_recv(rq_packet.buffer, rq_packet.len, buffer, RQ_PACKETSIZE);
    this->react_to_first_response(buffer);

    char file_buffer[this->blocksize];
    std::streamsize bytes_read;
    while (std::cin.read(file_buffer, this->blocksize) || std::cin.gcount() > 0) {
        bytes_read = std::cin.gcount();
        DATA_packet data_packet(++this->blockid, file_buffer, bytes_read);

        n = this->send_and_recv(data_packet.buffer, data_packet.len, 
                                buffer, RQ_PACKETSIZE);
        // check what we got from server
        Opcode op = Utils::get_opcode(buffer, 0);
        switch(op) {
            case Opcode::ACK:
            {
                ACK_packet ack_packet(buffer);
                logger.log_packet(&ack_packet, this->src);
            }
                break;
            case Opcode::ERROR:
            {
                std::cout << "ERROR packet recived" << std::endl;
            }
                break;
            default:
                error_exit("CAN'T FIND SUITABLE OPCODE");
                break;
        }



        std::cin.clear();
    }
    // if last read chunk was exactly blocksize
    // send empty packet
    if (bytes_read == BLOCKSIZE) { 
        this->send_empty_data_packet_recv_ack();
    }
}

void Client::setTimeout(int seconds)
{
    this->curr_timeout = seconds;
    this->timeout.tv_sec = seconds;
    this->timeout.tv_usec = 0;

	setsockopt(this->sock, SOL_SOCKET, SO_RCVTIMEO, &this->timeout, sizeof(timeval));
}

void Client::run() {

    // check port limits
    if (this->port < 0 || this->port > 65535) {
        error_exit("wrong port");
    }

    memset(&(this->server),0,sizeof(this->server));
    this->server.sin_family = AF_INET;

    if((this->servent = gethostbyname(this->hostname.c_str())) == NULL) {
        error_exit("wrong host");
    }

    memcpy(&(this->server.sin_addr), this->servent->h_addr, this->servent->h_length);
    this->server.sin_port = htons(this->port);
    // create socket
    if((this->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        error_exit("socket creation error");
    }

    // set default timeout
    this->setTimeout(this->default_timeout);
    // set source ip and port
    this->src = Utils::find_src(&(this->server));

    if(this->opcode == Opcode::WRQ){
        this->WRQ();
    } else {
        // RRQ();
    }
}
