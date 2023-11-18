#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <fstream>
#include <thread>
#include <random>
#include <sstream>
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
    
    //check if the packet is from the same client
    ip_t incoming_src = Utils::find_src(dest);
    if (incoming_src.port != cfg->src.port || strcmp(incoming_src.ip, cfg->src.ip) != 0) {
        this->send_error_packet(Error::UNKNOWN_TID, "Unknown transfer ID", cfg);
        return -1;
    }
    return n;
}

void Config::setTimeout(int seconds)
{
    this->curr_timeout = seconds;
    this->timeout.tv_sec = seconds;
    this->timeout.tv_usec = 0;

	setsockopt(this->sock, SOL_SOCKET, SO_RCVTIMEO, &this->timeout, sizeof(timeval));
}



void Server::change_from_netascii(Config *cfg) {
    std::string filepath = this->root_dirpath + "/" + cfg->filename;
    std::ifstream infile(filepath, std::ios::binary);
    std::stringstream buffer;
    buffer << infile.rdbuf();
    std::string file_content = buffer.str();
    std::ofstream outfile(filepath, std::ios::binary);
    Utils::convert_string_from_netascii(&file_content);
    outfile.write(file_content.c_str(), file_content.size());
    infile.close();
    outfile.close();
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

void Server::respond_to_rrq_rq(Config *cfg) {
    int n;
    if (cfg->opt_mode) { 
        std::string new_ts = std::to_string(cfg->filesize);
        Server::change_option_if_exists(&cfg->options, "tsize", new_ts); 
        OACK_packet oack_packet(cfg->options);
        cfg->len = sizeof(cfg->client);
        n = send(cfg, &cfg->client, oack_packet.buffer, oack_packet.len);
    }
}

void Server::send_empty_data_packet_recv_ack(Config *cfg) {
    Logger logger;
    char buffer[RQ_PACKETSIZE] = {0};
    DATA_packet data_packet(++cfg->blockid, nullptr, 0);
    int n;
    n = send(cfg, &cfg->client, data_packet.buffer, data_packet.len);
    n = recv(cfg, &cfg->client, buffer, SERVER_BLOCKSIZE+4);
    ACK_packet ack_packet(buffer);
    logger.log_packet(&ack_packet, cfg->src);
}
std::string Server::read_file_into_string(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cout << "Failed to open the file." << std::endl;
        // error ...
        return "";
    }
    std::string fileContent;
    file.seekg(0, std::ios::end);
    fileContent.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&fileContent[0], fileContent.size());
    file.close();
    return fileContent;
}

void Server::netascii_rrq(Config *cfg) {
    std::string filepath = this->root_dirpath + "/" + cfg->filename;
    std::string file_content = this->read_file_into_string(filepath);
    Utils::convert_string_to_netascii(&file_content);
    ssize_t file_len = file_content.length();
    ssize_t curr_size = 0;

    char file_buffer[cfg->blocksize];
    bool send_empty_packet = false;

    int iter = 0;
    int bytes_read = 0;
    for (curr_size = 0; curr_size < file_len; curr_size++){
        file_buffer[iter] = file_content[curr_size]; 
        bytes_read++;
        iter++;
        if (iter == cfg->blocksize || curr_size == file_len-1) {
            DATA_packet data_packet(++cfg->blockid, file_buffer, bytes_read);
            char buffer[RQ_PACKETSIZE] = {0};

            int n = send(cfg, &cfg->client, data_packet.buffer, data_packet.len);
            n = recv(cfg, &cfg->client, buffer, SERVER_BLOCKSIZE+4);

            //check response from client
            Opcode op = Utils::get_opcode(buffer, 0);
            switch(op) {
                case Opcode::ACK:
                {
                    ACK_packet ack_packet(buffer);
                    cfg->logger.log_packet(&ack_packet, cfg->src);
                }
                    break;
                case Opcode::ERROR:
                {
                    ERROR_packet error_packet(buffer);
                    cfg->logger.log_packet(&error_packet, cfg->src, cfg->dest);
                    exit(0);
                }
                    break;
                default:
                    error_exit("CAN'T FIND SUITABLE OPCODE");
                    break;
            }
            if (iter == cfg->blocksize && curr_size == file_len-1) {
                send_empty_packet = true;
            }
            iter = 0;          
            bytes_read = 0;
        }
        if (send_empty_packet) {
            this->send_empty_data_packet_recv_ack(cfg);
        }
    }
}

void Server::RRQ(Config *cfg) {
    char buffer[SERVER_BLOCKSIZE + 4] = {0};
    std::string filepath = this->root_dirpath + "/" + cfg->filename;

    if (!this->file_exists(filepath)) {
        this->send_error_packet(Error::FILE_NOT_FOUND, "File not found", cfg);
        return;
    } 
    std::ifstream infile(filepath, std::ifstream::binary);


    if (infile) {
        infile.seekg(0, infile.end);
        cfg->filesize = infile.tellg();
        infile.seekg(0, infile.beg);
    }

    this->respond_to_rrq_rq(cfg);

    int n;
    // if we have sent OACK, now recive ACK
    if (cfg->opt_mode) { 
        n = recv(cfg, &cfg->client, buffer, SERVER_BLOCKSIZE+4);
        ACK_packet ack_packet(buffer);
        cfg->logger.log_packet(&ack_packet, cfg->src);
        if (ack_packet.blockid != 0) {
            this->send_error_packet(Error::ILLEGAL_OPERATION, "Illegal operation", cfg);
            return;
        }
    }

    if (cfg->mode == "netascii") {
        this->netascii_rrq(cfg);
        return;
    }

    char file_buffer[cfg->blocksize] = {0};
    std::streamsize bytes_read;
    std::cout<<"blocksize: "<<cfg->blocksize<<std::endl;
    while (infile.read(file_buffer, cfg->blocksize) || infile.gcount() > 0) {
        bytes_read = infile.gcount();
        DATA_packet data_packet(++cfg->blockid, file_buffer, bytes_read);

        n = send(cfg, &cfg->client, data_packet.buffer, data_packet.len);
        n = recv(cfg, &cfg->client, buffer, SERVER_BLOCKSIZE+4);
        ACK_packet ack_packet(buffer);
        cfg->logger.log_packet(&ack_packet, cfg->src);
    }

    infile.close();
    if (bytes_read == cfg->blocksize) { 
        std::cout<<"sending empty data packet"<<std::endl;
        this->send_empty_data_packet_recv_ack(cfg);
    }
           
}

void Server::send_error_packet(Error errcode, std::string errmsg, Config *cfg) {
    ERROR_packet error_packet(errcode, errmsg);
    this->send(cfg, &cfg->client, error_packet.buffer, error_packet.len);
}

bool Server::file_exists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

void Server::WRQ(Config *cfg) {
    char buffer[SERVER_BLOCKSIZE + 4] = {0};
    int n;
    Logger logger;
    cfg->src = Utils::find_src(&cfg->client);
    this->respond_to_wrq_rq(cfg);
    std::string filepath = this->root_dirpath + "/" + cfg->filename;
    if (this->file_exists(filepath)) {
        this->send_error_packet(Error::FILE_ALREADY_EXISTS, "File already exists", cfg);
        return;
    }

    std::ofstream outfile(filepath, std::ios::binary);
    if (!outfile.is_open()) {
        this->send_error_packet(Error::ACCESS_VIOLATION, "Access violation", cfg);
        return;
    }

    while (true) {
        n = recv(cfg, &cfg->client, buffer, SERVER_BLOCKSIZE+4);
        int buffer_len = n;
        DATA_packet data_packet(buffer, n);
        logger.log_packet(&data_packet, cfg->src, cfg->dest);


        outfile.write(data_packet.data, data_packet.data_size);

        ACK_packet ack_packet(++cfg->blockid);
        n = send(cfg, &cfg->client, ack_packet.buffer, ack_packet.len);

        if (buffer_len-4 < SERVER_BLOCKSIZE) {
            outfile.close();
            break;
        }
    }
    if (cfg->mode == "netascii") {
       this->change_from_netascii(cfg); 
    }
}

void Server::change_option_if_exists(std::vector<option_t> *options, std::string name, std::string value) {
    for (auto &opt : *options) {
        if (opt.name == name) {
            opt.value = value;
        }
    }
}

void Server::handle_client(sockaddr_in client_address, char recv_buffer[RQ_PACKETSIZE], int bytes_read) {
    // create new socket and bind it with address
    Config *cfg = new Config();
    if ((cfg->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        error_exit("socket creation failed");
    }
    
    cfg->server.sin_family = AF_INET;
    cfg->server.sin_addr.s_addr = htonl(INADDR_ANY);  
    cfg->server.sin_port = 0;

    if (bind(cfg->sock, (struct sockaddr *)&cfg->server, sizeof(cfg->server)) < 0) {
        error_exit("bind error\n");
    }

    socklen_t addrlen = sizeof(cfg->server);
    getsockname(cfg->sock, (struct sockaddr*)&cfg->server, &addrlen);

    cfg->dest.port = ntohs(cfg->server.sin_port);
    std::cout << "New client (communicating on port: "<<cfg->dest.port<<")" << std::endl;
    cfg->client = client_address;
    RQ_packet rq_packet(recv_buffer);
    cfg->src = Utils::find_src(&cfg->client);
    cfg->logger.log_packet(&rq_packet, cfg->src);

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
        this->RRQ(cfg);
    } 
    else {
        error_exit("Invalid opcode");
    }


    std::cout <<"Client ("<<cfg->dest.port<<") done" << std::endl;
    close(cfg->sock);
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

        char send_buffer[RQ_PACKETSIZE] = {0};
        memcpy(send_buffer, buffer, RQ_PACKETSIZE);
        std::thread clientThread(&Server::handle_client, this, client_address, 
                                send_buffer, bytes_read);
        clientThread.detach();
    } 
    close(serverSock);
}
