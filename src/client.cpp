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
    // is hostname adress or domain
}

void Client::print_status() {
    std::cout << "hostname: " << hostname << std::endl;
    std::cout << "port: " << port << std::endl;
    std::cout << "filepath: " << server_filepath << std::endl;
    std::cout << "dest_path: " << dest_path << std::endl;
}

void Client::WRQ() {
    // ...
}

void Client::run() {
    struct hostent *server;

    // get host (supporting IPv4 only)
    if((inet_pton(AF_INET, hostname.c_str(), &addr.sin_addr)) <= 0){
        error_exit("wrong host");
    }

    // check port
    if (port < 0 || port > 65535) {
        error_exit("wrong port");
    }

    // specify IPv4 and set port
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    // create socket
    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        error_exit("socket creation error");
    }

    // set timeout
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0){
        error_exit("setsockopt error");
    }
}