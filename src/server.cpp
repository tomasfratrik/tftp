#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <fstream>
#include <thread>
#include <random>
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

// int Server::recv(Config *cfg, struct sockaddr_in *dest, char *buffer, int len){
//     cfg->len = sizeof(cfg->client);
//     int n = recvfrom(cfg->sock, buffer, len, 0, 
//             (struct sockaddr *)dest, (socklen_t *)&(cfg->len));

//     if (n < 0) {
//         error_exit("recvfrom error");
//     }
//     return n;
// }
int Server::recv(Config *cfg, struct sockaddr_in *dest, char *buffer, int buffer_len){
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(cfg->sock, &read_fds);

    sockaddr_in inaddr;
    cfg->len = sizeof(cfg->client);
    int sel = select(cfg->sock + 1, &read_fds, nullptr, nullptr, &cfg->timeout);
    if (sel < 0) {
        // differ from possible -1 from recvfrom
        return -2;
    }

    int n = recvfrom(cfg->sock, buffer, buffer_len, 0, 
            (struct sockaddr *)dest, (socklen_t *)&(cfg->len));
    return n;
}

void Config::setTimeout(int seconds)
{
    this->curr_timeout = seconds;
    this->timeout.tv_sec = seconds;
    this->timeout.tv_usec = 0;

	setsockopt(this->sock, SOL_SOCKET, SO_RCVTIMEO, &this->timeout, sizeof(timeval));
}


void Server::respond_to_wrq_rq(Config *cfg){
    int n;
    if (!cfg->opt_mode) { //just send ack if no options
        ACK_packet ack_packet(0);
        cfg->len = sizeof(cfg->client);
        n = send(cfg, &cfg->client, ack_packet.buffer, ack_packet.len);
    }
    else {
        OACK_packet oack_packet(cfg->options);
        cfg->len = sizeof(cfg->client);
        n = send(cfg, &cfg->client, oack_packet.buffer, oack_packet.len);
    }
}

void Server::WRQ(Config *cfg){
    char buffer[SERVER_BLOCKSIZE + 4] = {0};
    int n;
    Logger logger;
    cfg->src = Utils::find_src(&cfg->client);
    this->respond_to_wrq_rq(cfg);

    while (true) {
        n = recv(cfg, &cfg->client, buffer, SERVER_BLOCKSIZE+4);
        int buffer_len = n;
        DATA_packet data_packet(buffer, n);
        logger.log_packet(&data_packet, cfg->src, cfg->dest);

        std::string filepath = this->root_dirpath + "/" + cfg->filename;
        std::ofstream outfile(filepath, std::ios::binary | std::ios::app);
        // std::ofstream outfile(filepath, std::ios::binary);
        if (!outfile.is_open()) {
            error_exit("Could not open file");
        }
        // outfile.write(buffer, n-4);
        outfile.write(data_packet.data, data_packet.data_size);

        // cfg->blockid++;
        ACK_packet ack_packet(++cfg->blockid);
        n = send(cfg, &cfg->client, ack_packet.buffer, ack_packet.len);

        if (buffer_len-4 < SERVER_BLOCKSIZE) {
            break;
            outfile.close();
        }
    }
}

void Server::handle_client(sockaddr_in client_address, char recv_buffer[RQ_PACKETSIZE], int bytes_read) {
    // randomly generate new port
    // TODO: can't we generate with assigning 0, so os will assign random port?
    std::random_device rd;
    std::default_random_engine generator(rd());
    std::uniform_int_distribution<int> distribution(10000, 65535);
    int new_port = distribution(generator);

    // create new socket and bind it with address
    Config *cfg = new Config();
    if ((cfg->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        error_exit("socket creation failed");
    }
    
    cfg->server.sin_family = AF_INET;
    cfg->server.sin_addr.s_addr = htonl(INADDR_ANY);  
    cfg->server.sin_port = htons(new_port);
    cfg->dest.port = new_port;

    if (bind(cfg->sock, (struct sockaddr *)&cfg->server, sizeof(cfg->server)) < 0) {
        error_exit("bind\n (port probably already in use)");
    }

    cfg->client = client_address;
    RQ_packet rq_packet(recv_buffer);
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
    cfg->setTimeout(DEFAULT_TIMEOUT);

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

void Server::run(){
    
    // cerate server socket
    int serverSock;
    if ((serverSock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        error_exit("socket creation failed");
    }

    // create server address
    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);  
    server_address.sin_port = htons(this->port);
    

    // bind socket and address
    if (bind(serverSock, reinterpret_cast<sockaddr*>(&server_address), sizeof(server_address)) < 0) {
        error_exit("binding socket");
    }

    // handle incoming clients, and create a new thread for each client
    while(true){
        char buffer[RQ_PACKETSIZE] = {0};
        sockaddr_in client_address{};
        socklen_t client_address_len = sizeof(client_address);

        int bytes_read = recvfrom(serverSock, buffer, RQ_PACKETSIZE, 0,
                            reinterpret_cast<sockaddr*>(&client_address), &client_address_len);
        buffer[bytes_read] = '\0';

        if (bytes_read < 0) {
            error_exit("recvfrom error, bytes read < 0");
        }
        std::cout << "New incoming client" << std::endl;

        char send_buffer[RQ_PACKETSIZE] = {0};
        memcpy(send_buffer, buffer, RQ_PACKETSIZE);
        std::thread clientThread(&Server::handle_client, this, client_address, 
                                send_buffer, bytes_read);
        clientThread.detach();
    } 
}
