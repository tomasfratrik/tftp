/**
 * @file argc.hpp
 * @author Tomas Fratrik (xfratr01)
 * @brief header  
 * 
 */

#ifndef __ARGS_C_HPP__
#define __ARGS_C_HPP__
#include <string>
#include <netinet/in.h>
#include "./tftp.hpp"

class Args {
    public:
        std::string hostname;
        struct sockaddr_in address;
        int port = DEFAULT_PORT;
        std::string filepath;
        std::string dest_path;
        Args(int argc, char** argv);
};


#endif // __ARGC_HPP__