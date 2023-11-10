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


void Server::run(){
    std::cout << "Server running on port " << port << std::endl;
    std::cout << "Root directory: " << root_dirpath << std::endl;
    int opt = 1;
    char buffer[RQ_PACKETSIZE];
    int sock;
    struct sockaddr_in server_addr, client_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  
    server_addr.sin_port = htons(port);

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        error_exit("socket creation failed");
    }

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error_exit("bind\n (port probably already in use)");
    }

    int len = sizeof(client_addr);
    int n;
    while (1) {
        n = recvfrom(sock, (char *)buffer, RQ_PACKETSIZE, MSG_WAITALL, (struct sockaddr *)&client_addr, (socklen_t *)&len);
        buffer[n] = '\0';

        RQ_packet rq_packet(buffer);
        ip_t src = Utils::find_src(&client_addr);
        logger.log_packet(&rq_packet, src);

        ACK_packet ack_packet(Opcode::ACK, 0);

        sendto(sock, ack_packet.buffer, ack_packet.len, MSG_CONFIRM, (const struct sockaddr *)&client_addr, len);
    }

}
