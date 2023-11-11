#include <iostream>
#include "./args-server.hpp"
#include "./server.hpp"
#include "./tftp-server.hpp"

int main(int argc, char** argv){
    Args_server args(argc, argv);
    Server *server = new Server(&args);
    server->run();
    return 0;
}
