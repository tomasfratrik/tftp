/**
 * @file argc.hpp
 * @author Tomas Fratrik (xfratr01)
 * @brief header  
 * 
 */

#ifndef __ARGC_HPP__
#define __ARGC_HPP__
#include <string>
#include <netinet/in.h>

class Args {
    public:
        std::string hostname;
        struct sockaddr_in address;
        int port = 69;
        std::string filepath;
        std::string dest_path;
        Args(int argc, char** argv);
};


#endif // __ARGC_HPP__