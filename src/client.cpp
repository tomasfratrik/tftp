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

void Client::send_wrq_packet() {
    RQ_packet rq_packet(this->opcode, "test.txt", this->mode);
    rq_packet.init_buffer();
    int n = this->send(rq_packet.buffer, rq_packet.len);
}

void Client::WRQ() {
    int msg_size;
    char buffer[PACKETSIZE] = {0};
    int n;
    this->send_wrq_packet();
    memset(buffer, 0, PACKETSIZE);
    n = this->recv(buffer, PACKETSIZE);

    ACK_packet::print_buffer(buffer);

    // while(msg_size = read(STDIN_FILENO, buffer, PACKETSIZE)) {

}

void Client::run() {

    // check port
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