#include<stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "./tftp.hpp"
#include "./packet.hpp"


ip_t Utils::find_src(struct sockaddr_in *addr) {
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr->sin_addr), client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(addr->sin_port);
    ip_t src;
    strcpy(src.ip, client_ip);
    src.port = client_port;
    return src;
}

Opcode Utils::get_opcode(char *buffer, int i) {
    return (Opcode)ntohs(*(uint16_t*)(&buffer[i]));
}

int Utils::get_2byte_num(char *buffer, int i) {
    try {
        int opcode = ntohs(*(uint16_t*)(&buffer[i]));
        return opcode;
    }
    catch(int err){
        return -1;
    }
}

void Utils::set_2byte_num(char *buffer, int i, int num) {
    *(uint16_t*)(&buffer[i]) = htons(num);
}

void Utils::replace_from_to(std::string *string, std::string from, std::string to)
{
    ssize_t pos = 0;
    while(true) {
		pos = (*string).find(from, pos);
		if (pos == std::string::npos) {
			break;
		}
		(*string).erase(pos, from.length());
		(*string).insert(pos, to);
        pos += to.length();
    }
}

void Utils::convert_string_to_netascii(std::string *string) {
    Utils::replace_from_to(&(*string), "\r", "\r\0");
	Utils::replace_from_to(&(*string), "\n", "\r\n");
}

void Utils::convert_string_from_netascii(std::string *string) {
    Utils::replace_from_to(&(*string), "\r\0", "\r");
	Utils::replace_from_to(&(*string), "\r\n", "\n");
}

void Utils::print_string_LFCR(std::string string) {
    for (char c : string) {
        if(c == '\0') {
            std::cout << "\\0";
        } else if (c == '\r') {
            std::cout << "\\r";
        } else if (c == '\n') {
            std::cout << "\\n";
        } else {
            std::cout << c;
        }
    }

}

std::string Utils::convert_mode_to_str(Mode mode) {
    switch(mode) {
        case Mode::NETASCII:
            return "netascii";
        case Mode::OCTET:
            return "octet";
        case Mode::MAIL:
            return "mail";
        default:
            return "";
    }
}

