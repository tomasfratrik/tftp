#ifndef __TFTP__
#define __TFTP__
#include "./packet.hpp"

#define DEFAULT_PORT 69
#define DEFAULT_TIMEOUT 1
#define MAX_TIMEOUT_TRIES 3 

class Utils {
    public:
        static int get_2byte_num(char *buffer, int pos);
        static ip_t find_src(struct sockaddr_in *addr);
        static Opcode get_opcode(char *buffer, int pos);
        static std::string convert_mode_to_str(Mode mode);
        static void set_2byte_num(char *buffer, int pos, int num);
        static void print_string_LFCR(std::string string);
        static void replace_from_to(std::string *string, std::string from, std::string to);
        static void convert_string_to_netascii(std::string *string);
        static void convert_string_from_netascii(std::string *string);
};


#endif // __TFTP__
