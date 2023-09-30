#include <iostream>
#include "./args-client.hpp"
#include "./packet.hpp"
#include "./client.hpp"

int main(int argc, char** argv){
    Args args(argc, argv);
    Client client(&args);
    client.run();
    // delete &args;
    // delete &client;
    return 0;
}