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
    filepath = args->filepath;
    dest_path = args->dest_path;
    // is hostname adress or domain
}

void Client::print_status(){
    // std::cout << "hostname: "j << (std::string)addr << std::endl;
    std::cout << "hostname: " << hostname << std::endl;
    std::cout << "port: " << port << std::endl;
    std::cout << "filepath: " << filepath << std::endl;
    std::cout << "dest_path: " << dest_path << std::endl;
}

void Client::run(){
    struct hostent *server;

    if((server = gethostbyname(hostname.c_str())) == NULL){
        error_exit("wrong host");
    }
    Client::print_status();
}