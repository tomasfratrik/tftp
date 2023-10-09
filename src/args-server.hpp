#ifndef __ARGS_S_HPP__
#define __ARGS_S_HPP__
#include <string>

class Args_server {
    public:
        int port = 1337;
        std::string root_dirpath;
        Args_server(int argc, char** argv);
};

#endif // __ARGS_S_HPP__