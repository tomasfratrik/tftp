#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <fstream>
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

int Server::send(Config *cfg, struct sockaddr_in *dest, char *buffer, int len){
    int n = sendto(cfg->sock, buffer, len, 0, 
            (struct sockaddr *)dest, sizeof(*dest));

    if (n < 0) {
        error_exit("sendto error");
    }
    return n;

}

int Server::recv(Config *cfg, struct sockaddr_in *dest, char *buffer, int len){
    cfg->len = sizeof(cfg->client);
    int n = recvfrom(cfg->sock, buffer, len, 0, 
            (struct sockaddr *)dest, (socklen_t *)&(cfg->len));

    if (n < 0) {
        error_exit("recvfrom error");
    }
    return n;
}

void Server::respond_to_wrq_rq(Config *cfg){
    int n;
    if (!cfg->opt_mode) { //just send ack
        ACK_packet ack_packet(0);
        cfg->len = sizeof(cfg->client);
        n = send(cfg, &cfg->client, ack_packet.buffer, ack_packet.len);
    }
    else {
        // print options
        OACK_packet oack_packet(cfg->options);
        cfg->len = sizeof(cfg->client);
        n = send(cfg, &cfg->client, oack_packet.buffer, oack_packet.len);
    }
}

void Server::WRQ(Config *cfg){
    char buffer[SERVER_BLOCKSIZE + 4] = {0};
    int n;
    Logger logger;
    this->respond_to_wrq_rq(cfg);

    while (true) {
        n = recv(cfg, &cfg->client, buffer, SERVER_BLOCKSIZE+4);
        int buffer_len = n;
        DATA_packet data_packet(buffer, n);
        ip_t src = Utils::find_src(&cfg->client);
        logger.log_packet(&data_packet, src);

        std::string filepath = this->root_dirpath + "/" + cfg->filename;
        std::ofstream outfile(filepath, std::ios::binary | std::ios::app);
        // std::ofstream outfile(filepath, std::ios::binary);
        if (!outfile.is_open()) {
            error_exit("Could not open file");
        }
        // outfile.write(buffer, n-4);
        outfile.write(data_packet.data, data_packet.data_size);

        // cfg->blockid++;
        ACK_packet ack_packet(cfg->blockid);
        n = send(cfg, &cfg->client, ack_packet.buffer, ack_packet.len);

        if (buffer_len-4 < SERVER_BLOCKSIZE) {
            break;
            outfile.close();
        }
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
    int n = recv(cfg, &cfg->client, buffer, RQ_PACKETSIZE);

    buffer[n] = '\0';
    RQ_packet rq_packet(buffer);
    ip_t src = Utils::find_src(&cfg->client);
    cfg->logger.log_packet(&rq_packet, src);

    if (rq_packet.options.empty()) {
        cfg->opt_mode = false;
    } else {
        cfg->opt_mode = true;
        for (auto opt : rq_packet.options) {
            if (opt.name == "blksize") {
                opt.value = "1024";
                cfg->options.push_back(opt);
            }
            else if (opt.name == "tsize") {
                opt.value = "1";
                cfg->options.push_back(opt);
            }
            else if (opt.name == "timeout") {
                opt.value = "1";
                cfg->options.push_back(opt);
            }
        }
    }
    cfg->filename = rq_packet.filename;
    cfg->mode = rq_packet.mode;



    if (rq_packet.opcode == Opcode::WRQ) {
        this->WRQ(cfg);
    } 
    else if (rq_packet.opcode == Opcode::RRQ) {
        // cfg->RRQ();
    } 
    else {
        error_exit("Invalid opcode");
    }


}
