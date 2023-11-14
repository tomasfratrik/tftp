#ifndef __ARGS_S_HPP__
#define __ARGS_S_HPP__
#include <string>
#include "./tftp.hpp"

class Args_server {
    public:
        int port = DEFAULT_PORT;
        std::string root_dirpath;
        Args_server(int argc, char** argv);
};

#endif // __ARGS_S_HPP__