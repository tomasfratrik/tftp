#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstring>
#include "./client.hpp"
#include "./error_exit.hpp"
#include "./tftp.hpp"
#include "./packet.hpp"
#include "./logger.hpp"



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
    blocksize = BLOCKSIZE;
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
    int n = recvfrom(this->sock, buffer, len, 0, 
            (struct sockaddr *)&(this->server), &(this->len));

    if (n < 0){
        error_exit("recvfrom error");
    }
    return n;
}

void Client::send_rq_packet() {
    option_t blksize_opt = {.name = "blksize", .value = "1024"};
    option_t test = {.name = "tsize", .value = "1"};
    this->options.push_back(blksize_opt);
    this->options.push_back(test);

    RQ_packet rq_packet(this->opcode, this->dest_path, this->mode, this->options);
    int n = this->send(rq_packet.buffer, rq_packet.len);
}


void Client::WRQ() {
    char buffer[RQ_PACKETSIZE] = {0};
    Logger logger;

    this->send_rq_packet();
    memset(buffer, 0, RQ_PACKETSIZE);
    int n = this->recv(buffer, RQ_PACKETSIZE);
    Opcode op = Utils::get_opcode(buffer, 0);

    /**
     * Only 3 types of respones from server
     * are possible to our write request
     */
    ip_t src = Utils::find_src(&(this->server));
    switch(op) {
        case Opcode::ACK:
        {
            ACK_packet ack_packet(buffer);
            logger.log_packet(&ack_packet, src);
        }
            break;
        case Opcode::OACK:
        {
            std::cout << "OACK" << std::endl;
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

    if(opcode == Opcode::WRQ){
        this->WRQ();
    } else {
        // RRQ();
    }
}
