#include <iostream>
#include <unistd.h>
#include "./error_exit.hpp"
#include "./packet.hpp"

void print_usage(){
    std::cout << 
    "Usage: ./tftp-client -h hostname [-p port] [-f filepath] -t dest_path\n" 
    "Options:\n"
    "\t -h hostname: hostname of server\n"
    "\t -p port: port to connect to\n"
    "\t\t *not specified: use default port\n"
    "\t -f filepath: path to file to send\n"
    "\t\t *specified: READ that file from server\n"
    "\t\t *not specified: WRITE to server (from stdin)\n"
    "\t -t dest_path: where will be file saved (localy or on server: depends on -f)\n)"
    << std::endl;
}

void print_usage_server(){
    std::cout << 
    "Usage: ./tftp-server [-p port] root_dirpath\n" 
    "Options:\n"
    "\t -p port: port to listen on\n"
    "\t\t *not specified: use default port\n"
    "\t root_dirpath: directory to serve files from"
    << std::endl;
}

void error_exit(std::string msg){
    std::cout << "ERROR: " << msg << std::endl;
    exit(1);
}