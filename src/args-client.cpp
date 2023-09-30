#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include "./args-client.hpp"
#include "./error_exit.hpp"

Args::Args(int argc, char** argv) {
    int opt;

    while((opt = getopt(argc, argv, "h:p:f:t:")) != -1){
        switch(opt) {
            case 'h':
                hostname = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'f':
                filepath = optarg;
                break;
            case 't':
                dest_path = optarg;
                break;
            default:
                print_usage();
                error_exit("Invalid arguments");
        }
    }

    if(hostname.empty() || dest_path.empty()){
        print_usage();
        error_exit("hostname or dest_path not specified");
    }

}


