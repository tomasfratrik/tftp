#include<stdio.h>
#include "./tftp.hpp"
#include "./packet.hpp"

int get_2byte_num(char *buffer, int pos) {
    try {
        int opcode = ntohs(*(uint16_t*)(&buffer[pos]));
        return opcode;
    }
    catch(int err){
        return -1;
    }
}