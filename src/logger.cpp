#include "./logger.hpp"
#include "./tftp.hpp"
#include <cstring>

void Logger::log_packet(RQ_packet *packet, ip_t src){
        if (packet->opcode == Opcode::RRQ){
            std::cerr<<"RRQ ";
        } else if (packet->opcode == Opcode::WRQ){
            std::cerr<<"WRQ ";
        }
        std::cerr<<src.ip<<":"<<src.port<<" ";
        std::cerr<<"\""<<packet->filename<<"\""<<" ";
        std::cerr<<packet->mode;
        for (auto opt : packet->options){
            std::cerr<<" "<<opt.name<<"="<<opt.value;
        }
        std::cerr<<std::endl;
}

void Logger::log_packet(ACK_packet *packet, ip_t src){
        std::cerr<<"ACK ";
        std::cerr<<src.ip<<":"<<src.port<<" ";
        std::cerr<<packet->blockid<<std::endl;
}

void Logger::log_packet(OACK_packet *packet, ip_t src){
    std::cerr<<"OACK ";
    std::cerr<<src.ip<<":"<<src.port;
    for (auto opt : packet->options){
        std::cerr<<" "<<opt.name<<"="<<opt.value;
    }
    std::cerr<<std::endl;
}

void Logger::log_packet(DATA_packet *packet, ip_t src, ip_t dest) {
    std::cerr<<"DATA ";
    std::cerr<<src.ip<<":"<<src.port<<":"<<dest.port<<" ";
    std::cerr<<packet->blockid<<std::endl;
}

void Logger::log_packet(ERROR_packet *packet, ip_t src, ip_t dest) {
    std::cerr<<"ERROR ";
    std::cerr<<src.ip<<":"<<src.port<<":"<<dest.port<<" ";
    std::cerr<<(int)packet->error_code<<" "<<"\""<<packet->error_msg<<"\""<<std::endl;
}