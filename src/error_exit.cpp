#include <iostream>
#include <unistd.h>
#include "./error_exit.hpp"

void print_usage(){
    std::cerr << 
    "Usage: ./tftp-client -h hostname [-p port] [-f filepath] -t dest_path" 
    << std::endl;
}

void error_exit(std::string msg){
    std::cerr << "ERROR: " << msg << std::endl;
    exit(1);
}