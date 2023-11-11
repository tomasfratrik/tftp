#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "./server.hpp"
#include "./args-server.hpp"
#include "./packet.hpp"
#include "./error_exit.hpp"

Server::Server(Args_server *args) {
    port = args->port;
    root_dirpath = args->root_dirpath;
}

void Server::WRQ(){

    // test
}


void Server::run(){
    // std::cout << "Server running on port " << port << std::endl;
    // std::cout << "Root directory: " << root_dirpath << std::endl;
    int opt = 1;
    char buffer[RQ_PACKETSIZE];
    // int sock;

    this->server.sin_family = AF_INET;
    this->server.sin_addr.s_addr = htonl(INADDR_ANY);  
    this->server.sin_port = htons(port);

    if ((this->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        error_exit("socket creation failed");
    }

    if (bind(this->sock, (struct sockaddr *)&this->server, sizeof(this->server)) < 0) {
        error_exit("bind\n (port probably already in use)");
    }
    // if (bind(sock, (struct sockaddr *)&(this->this->client), sizeof(this->client)) < 0) { //     error_exit("bind\n (port probably already in use)");
    // }

    // int len = sizeof(this->this->client);
    int len = sizeof(this->client);
    int n;
    while (1) {
        n = recvfrom(this->sock, (char *)buffer, RQ_PACKETSIZE, MSG_WAITALL, (struct sockaddr *)&this->client, (socklen_t *)&len);
        buffer[n] = '\0';

        RQ_packet rq_packet(buffer);
        ip_t src = Utils::find_src(&this->client);
        logger.log_packet(&rq_packet, src);

        ACK_packet ack_packet(Opcode::ACK, 0);

        sendto(this->sock, ack_packet.buffer, ack_packet.len, MSG_CONFIRM, (const struct sockaddr *)&this->client, len);
    }

    
    

}
