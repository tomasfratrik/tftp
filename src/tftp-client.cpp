#include <iostream>
#include "./args-client.hpp"
#include "./client.hpp"
#include "./tftp-client.hpp"

int main(int argc, char** argv){
    Args args(argc, argv);
    Client client(&args);
    client.run();
    return 0;
}