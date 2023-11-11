#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "./server.hpp"
#include "./args-server.hpp"
#include "./packet.hpp"
#include "./error_exit.hpp"

Server::Server(Args_server *args) {
    this->port = args->port;
    this->root_dirpath = args->root_dirpath;
}

Config::Config() {
    // ...
}

void Server::WRQ(Config *cfg){
    char buffer[RQ_PACKETSIZE];
    int n;

    if(!cfg->opt_mode){
        // send ack
        ACK_packet ack_packet(0);
        cfg->len = sizeof(cfg->client);
        sendto(cfg->sock, ack_packet.buffer, ack_packet.len, MSG_CONFIRM, 
                        (const struct sockaddr *)&cfg->client, cfg->len);
    }
    else {
        // send oack
        OACK_packet oack_packet(cfg->options);
        cfg->len = sizeof(cfg->client);
        sendto(cfg->sock, oack_packet.buffer, oack_packet.len, MSG_CONFIRM, 
                        (const struct sockaddr *)&cfg->client, cfg->len);
    }
    exit(0);

    ACK_packet ack_packet(0);
    sendto(cfg->sock, ack_packet.buffer, ack_packet.len, MSG_CONFIRM, 
                    (const struct sockaddr *)&cfg->client, cfg->len);

    while (1) {
        n = recvfrom(cfg->sock, (char *)buffer, RQ_PACKETSIZE, MSG_WAITALL, 
                    (struct sockaddr *)&cfg->client, (socklen_t *)&cfg->len);
        buffer[n] = '\0';

        // cfg->logger.log_packet(&rq_packet, src);

        cfg->len = sizeof(cfg->client);
        ACK_packet ack_packet(0);
        sendto(cfg->sock, ack_packet.buffer, ack_packet.len, MSG_CONFIRM, 
                        (const struct sockaddr *)&cfg->client, cfg->len);
    }
}


void Server::run(){
    char buffer[RQ_PACKETSIZE];


    Config *cfg = new Config();
    cfg->server.sin_family = AF_INET;
    cfg->server.sin_addr.s_addr = htonl(INADDR_ANY);  
    cfg->server.sin_port = htons(this->port);

    if ((cfg->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        error_exit("socket creation failed");
    }

    if (bind(cfg->sock, (struct sockaddr *)&cfg->server, sizeof(cfg->server)) < 0) {
        error_exit("bind\n (port probably already in use)");
    }

    // parse first request packet
    cfg->len = sizeof(cfg->client);
    int n = recvfrom(cfg->sock, (char *)buffer, RQ_PACKETSIZE, MSG_WAITALL, 
                    (struct sockaddr *)&cfg->client, (socklen_t *)&cfg->len);
    buffer[n] = '\0';
    RQ_packet rq_packet(buffer);
    ip_t src = Utils::find_src(&cfg->client);
    cfg->logger.log_packet(&rq_packet, src);
    //check for options

    if(rq_packet.options.empty()){
        cfg->opt_mode = false;
    } else {
        cfg->opt_mode = true;
        cfg->options = rq_packet.options;
    }
    cfg->filename = rq_packet.filename;
    cfg->mode = rq_packet.mode;



    if(rq_packet.opcode == Opcode::WRQ){
        this->WRQ(cfg);
    } 
    else if(rq_packet.opcode == Opcode::RRQ){
        // cfg->RRQ();
    } 
    else {
        error_exit("Invalid opcode");
    }


}
