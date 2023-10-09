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

void Client::WRQ() {
    int msg_size;
    char buffer[PACKETSIZE] = {0};
    int n;
    socklen_t len = sizeof(server);
    RQ_packet rq_packet(opcode, "test.txt", Mode::OCTET);
    rq_packet.init_buffer();
    std::cout<< "len: " << rq_packet.len << std::endl;
    std::cout<< "mode: " << rq_packet.mode << std::endl;
    rq_packet.print_buffer(rq_packet.buffer, rq_packet.len);
    std::cout << std::endl;

    while(msg_size = read(STDIN_FILENO, buffer, PACKETSIZE)) {



        n = sendto(sock, buffer, msg_size, 0, 
                (struct sockaddr *)&server, len);
        
        if(n < 0){
            error_exit("sendto error");
        }
        memset(buffer, 0, PACKETSIZE);

        n = recvfrom(sock, buffer, PACKETSIZE, 0, 
                (struct sockaddr *)&server, &len);
        if(n < 0){
            error_exit("recvfrom error");
        }
        std::cout << "Server: " << buffer << std::endl;
    }

}

void Client::run() {

    // check port
    if (port < 0 || port > 65535) {
        error_exit("wrong port");
    }

    memset(&server,0,sizeof(server));
    server.sin_family = AF_INET;

    if((servent = gethostbyname(hostname.c_str())) == NULL) {
        error_exit("wrong host");
    }

    memcpy(&server.sin_addr, servent->h_addr, servent->h_length);

    server.sin_port = htons(port);

    // create socket
    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        error_exit("socket creation error");
    }

    // set timeout
    // struct timeval tv;
    // tv.tv_sec = 2;
    // tv.tv_usec = 0;
    // if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0){
    //     error_exit("setsockopt error");
    // }

    if(opcode == Opcode::WRQ){
        Client::WRQ();
    } else {
        // RRQ();
    }
}