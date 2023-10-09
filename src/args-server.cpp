#include <iostream>
#include "./args-server.hpp"
#include "./error_exit.hpp"

Args_server::Args_server(int argc, char** argv) {
    if(argc == 2) {
        root_dirpath = argv[1];
    }
    else if(argc == 4) {
        if(std::string(argv[1]) == "-p") {
            port = atoi(argv[2]);
            root_dirpath = argv[3];
        }
        else {
            print_usage_server();
            error_exit("Invalid arguments");
        }
    }
    else {
        print_usage_server();
        error_exit("Invalid arguments");
    }
}