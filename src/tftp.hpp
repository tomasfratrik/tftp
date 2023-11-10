#ifndef __TFTP__
#define __TFTP__
#include "./packet.hpp"

#define DEFAULT_PORT 69

class Utils {
    public:
        static int get_2byte_num(char *buffer, int pos);
        static ip_t find_src(struct sockaddr_in *addr);
        static Opcode get_opcode(char *buffer, int pos);
        static std::string convert_mode_to_str(Mode mode);
        static void set_2byte_num(char *buffer, int pos, int num);
};

#endif // __TFTP__
