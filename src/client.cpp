#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sstream>
#include <fstream>
#include <cstring>
#include "./client.hpp"
#include "./error_exit.hpp"



Client::Client(Args *args){
    hostname = args->hostname;
    port = args->port;
    server_filepath = args->filepath;
    dest_path = args->dest_path;
    if(server_filepath.empty()){
        opcode = Opcode::WRQ;
    } else {
        opcode = Opcode::RRQ;
    }
    mode = Mode::OCTET;
}

int Client::send(char *buffer, int len){
    int n = sendto(this->sock, buffer, len, 0, 
            (struct sockaddr *)&(this->server), sizeof(this->server));
    
    if (n < 0){
        std::cout<<"n: "<< n << std::endl;
        error_exit("sendto error");
    }
    return n;
}

int Client::recv(char *buffer, int len){
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(this->sock, &read_fds);

    sockaddr_in server_address;
    this->len = sizeof(server_address);
    int sel = select(this->sock + 1, &read_fds, nullptr, nullptr, &this->timeout);
    if (sel < 0) {
        // differ from possible -1 from recvfrom
        return -2;
    }

    int n = recvfrom(this->sock, buffer, len, 0, 
            (struct sockaddr *) &server_address, &(this->len));

    int incoming_port = ntohs(server_address.sin_port);

    //if port changed again send error
    if (this->port_changed && this->port != incoming_port) {
        ERROR_packet error_packet(Error::UNKNOWN_TID, "PORT CHANGED");
        this->send(error_packet.buffer, error_packet.len);
        exit(0);
    }

    else if (this->port_changed == false) {
        this->port = ntohs(server_address.sin_port);
        this->server.sin_port = htons(this->port);
        this->src.port = this->port;
        this->port_changed = true;
    }

    return n;
}

void Client::send_empty_data_packet_recv_ack() {
    Logger logger;
    char buffer[RQ_PACKETSIZE] = {0};
    DATA_packet data_packet(++this->blockid, nullptr, 0);
    int n = this->send(data_packet.buffer, data_packet.len);
    n = this->recv(buffer, RQ_PACKETSIZE);
    Opcode op = Utils::get_opcode(buffer, 0);
    switch(op) {
        case Opcode::ACK:
        {
            ACK_packet ack_packet(buffer);
            logger.log_packet(&ack_packet, this->src);
        }
            break;
        case Opcode::ERROR:
        {
            ERROR_packet error_packet(buffer);
            logger.log_packet(&error_packet, this->src, this->dest);
            exit(0);
        }
            break;
        default:
            send_error_packet(Error::ILLEGAL_OPERATION, "CAN'T FIND SUITABLE OPCODE");
            break;
    }
}

int Client::send_and_recv(char *send_buffer, int send_len, 
                        char *recv_buffer, int recv_len) {
    int n;
    this->setTimeout(this->default_timeout);
    for (int retry = 0; retry <= MAX_TIMEOUT_TRIES; retry++) {
        if (retry == MAX_TIMEOUT_TRIES) {
            error_exit("TIMED OUT");
        }
        this->send(send_buffer, send_len);
        n = this->recv(recv_buffer, recv_len);
        if (n >= 0) {
            break;
        }
        int newTimeout = this->curr_timeout * 2;
        this->setTimeout(newTimeout);
    }
    this->setTimeout(this->default_timeout);
    return n;
}

void Client::send_error_packet(Error errcode, std::string errmsg) {
    ERROR_packet error_packet(errcode, errmsg);
    this->send(error_packet.buffer, error_packet.len);
}


void Client::validate_options(OACK_packet oack_packet) {
    int value;
    for (auto opt : oack_packet.options) {
        if (opt.name == "blksize") {
            // if value is less then 8 throw error
            try {
                value = std::stoi(opt.value);
            }
            catch(int err) {
                this->send_error_packet(Error::ILLEGAL_OPERATION, 
                        "blksize must be a number");
            }
            if (value < 8 || value > 65464) {
                this->send_error_packet(Error::ILLEGAL_OPERATION, 
                        "blksize must be greater than 8 and less than 65464");
            }
            this->blocksize = value;
        }
        else if (opt.name == "timeout") {
            try {
                    value = std::stoi(opt.value);
            }
            catch(int err) {
                this->send_error_packet(Error::OPTIONS, 
                        "timeout must be a number");
                exit(0);
            }
            if (value < 1 || value > 255) {
                this->send_error_packet(Error::OPTIONS, 
                        "timeout must be greater than 1 and less than 255");
                exit(0);
            }
            this->default_timeout = value;
            this->setTimeout(this->default_timeout);
        }
        else if (opt.name == "tsize") {
            try {
                value = std::stoi(opt.value);
            }
            catch(int err) {
                this->send_error_packet(Error::OPTIONS, 
                        "tsize must be a number");
                exit(0);
            }
        }
    }
}
void Client::react_to_first_response_rrq(char *buffer, int buffer_len, std::ofstream& output) {
    Opcode op = Utils::get_opcode(buffer, 0);
    if (this->opcode == Opcode::RRQ) {
        switch(op) {
            case Opcode::DATA:
            {
                this->blocksize = DEFAULT_BLOCKSIZE;
                DATA_packet data_packet(buffer, buffer_len);
                logger.log_packet(&data_packet, src, dest);
                output.write(data_packet.data, data_packet.data_size);
                ACK_packet ack_packet(++this->blockid);
                this->send(ack_packet.buffer, ack_packet.len);
                
                // if this first data packet was aslo last
                if (data_packet.data_size < this->blocksize) {
                    this->completed = true;
                }

            }
                break;
            case Opcode::OACK:
            {
                std::cout << "OACK packet recived" << std::endl;
                OACK_packet oack_packet(buffer);
                logger.log_packet(&oack_packet, src);
                this->validate_options(oack_packet);
                ACK_packet ack_packet(0);
                this->send(ack_packet.buffer, ack_packet.len);
            }
                break;
            case Opcode::ERROR:
            {
                if (this->retransimitted_request) {
                    ERROR_packet error_packet(buffer);
                    logger.log_packet(&error_packet, this->src, this->dest);
                    std::cout << "ERROR packet recived again, after retransimitted request" << std::endl;
                    exit(0);
                }
                this->retransimitted_request = true;
                ERROR_packet error_packet(buffer);
                logger.log_packet(&error_packet, this->src, this->dest);
                this->opt_mode = false;
                RQ_packet rq_packet(this->opcode, this->dest_path, this->mode, this->options);
                int n = this->send(rq_packet.buffer, rq_packet.len);
            }
                break;
            default:
                error_exit("CAN'T FIND SUITABLE OPCODE");
                break;
        }
    }
}
void Client::react_to_first_response_wrq(char *buffer) {
    Opcode op = Utils::get_opcode(buffer, 0);
    int value;

    if(this->opcode == Opcode::WRQ){
        switch(op) {
            case Opcode::ACK:
            {
                ACK_packet ack_packet(buffer);
                this->logger.log_packet(&ack_packet, src);
                this->blocksize = DEFAULT_BLOCKSIZE;
            }
                break;
            case Opcode::OACK:
            {
                OACK_packet oack_packet(buffer);
                logger.log_packet(&oack_packet, src);
                this->validate_options(oack_packet);
            }
                break;
            case Opcode::ERROR:
            {
                if (this->retransimitted_request) {
                    ERROR_packet error_packet(buffer);
                    logger.log_packet(&error_packet, this->src, this->dest);
                    std::cout << "ERROR packet recived again, after retransimitted request" << std::endl;
                    exit(0);
                }
                this->retransimitted_request = true;
                ERROR_packet error_packet(buffer);
                logger.log_packet(&error_packet, this->src, this->dest);
                this->opt_mode = false;
                RQ_packet rq_packet(this->opcode, this->dest_path, this->mode, this->options);
                int n = this->send(rq_packet.buffer, rq_packet.len);
            }
                break;
            default:
                error_exit("CAN'T FIND SUITABLE OPCODE");
                break;
        }
    }
    
}

void Client::netascii_wrq() {
    // Read the entire stdin into a string
    std::string input((std::istreambuf_iterator<char>(std::cin)), std::istreambuf_iterator<char>());
    Utils::convert_string_to_netascii(&input);
    ssize_t input_size = input.length();
    ssize_t curr_size = 0;

    char file_buffer[this->blocksize];
    bool send_empty_packet = false;

    int iter = 0;
    int bytes_read = 0;
    for (curr_size = 0; curr_size < input_size; curr_size++){
        file_buffer[iter] = input[curr_size]; 
        bytes_read++;
        iter++;
        if (iter == this->blocksize || curr_size == input_size-1) {
            DATA_packet data_packet(++this->blockid, file_buffer, bytes_read);
            char buffer[RQ_PACKETSIZE] = {0};
            int n = this->send_and_recv(data_packet.buffer, data_packet.len, 
                                buffer, RQ_PACKETSIZE);
            // check what we got from server
            Opcode op = Utils::get_opcode(buffer, 0);
            switch(op) {
                case Opcode::ACK:
                {
                    ACK_packet ack_packet(buffer);
                    logger.log_packet(&ack_packet, this->src);
                }
                    break;
                case Opcode::ERROR:
                {
                    ERROR_packet error_packet(buffer);
                    logger.log_packet(&error_packet, this->src, this->dest);
                    exit(0);
                }
                    break;
                default:
                    error_exit("CAN'T FIND SUITABLE OPCODE");
                    break;
            }
            if (iter == this->blocksize && curr_size == input_size-1) {
                send_empty_packet = true;
            }
            iter = 0;          
            bytes_read = 0;
        }
        if (send_empty_packet) {
            this->send_empty_data_packet_recv_ack();
        }
    }
}

void Client::RRQ() {
    char buffer[RQ_PACKETSIZE] = {0};
    int n;
    std::ofstream outfile(this->dest_path, std::ios::binary);

    //
    // Initialize Read Request packet, and send it
    //
    option_t blksize_opt = {.name = "blksize", .value = "1024"};
    option_t timeout_opt = {.name = "timeout", .value = "1"};
    option_t tsize_opt = {.name = "tsize", .value = "0"};
    this->options.push_back(blksize_opt);
    this->options.push_back(timeout_opt);
    this->options.push_back(tsize_opt);
    RQ_packet rq_packet(this->opcode, this->dest_path, this->mode, this->options);
    n = this->send_and_recv(rq_packet.buffer, rq_packet.len, buffer, RQ_PACKETSIZE);
    this->react_to_first_response_rrq(buffer, n, outfile);

    // if we got an error, we retransimmed the request
    // so now we need to react to that response again
    if (this->retransimitted_request) {
        this->react_to_first_response_rrq(buffer, n, outfile);
    }

    char data_buffer[this->blocksize + 4];

    while (!this->completed) {

        n = recv(data_buffer, this->blocksize + 4);
        int buffer_len = n;
        
        DATA_packet data_packet(data_buffer, n);
        logger.log_packet(&data_packet, this->src, this->dest);

        if (!outfile.is_open()) {
            error_exit("Could not open file");
        }

        outfile.write(data_packet.data, data_packet.data_size);

        ACK_packet ack_packet(++this->blockid);
        n = send(ack_packet.buffer, ack_packet.len);

        if (buffer_len-4 < this->blocksize) {
            outfile.close();
            break;
        }
    }
    outfile.close();

}

void Client::WRQ() {
    char buffer[RQ_PACKETSIZE] = {0};
    int n;
    Logger logger;
    // check how many bytes does stdin have
    std::cin.seekg(0, std::ios::end);
    std::size_t input_size = std::cin.tellg();
    std::cin.seekg(0, std::ios::beg);

    //
    // Initialize Write Request packet, and send it
    //
    option_t blksize_opt = {.name = "blksize", .value = "1024"};
    option_t timeout_opt = {.name = "timeout", .value = "1"};
    option_t tsize_opt = {.name = "tsize", .value = std::to_string(input_size)};
    this->options.push_back(blksize_opt);
    this->options.push_back(timeout_opt);
    this->options.push_back(tsize_opt);
    RQ_packet rq_packet(this->opcode, this->dest_path, this->mode, this->options);
    n = this->send_and_recv(rq_packet.buffer, rq_packet.len, buffer, RQ_PACKETSIZE);
    this->react_to_first_response_wrq(buffer);

    // if we got an error, we retransimmed the request
    // so now we need to react to that response again
    if (this->retransimitted_request) {
        this->react_to_first_response_wrq(buffer);
    }
    if (this->mode == Mode::NETASCII) {
        this->netascii_wrq();
    }

    char file_buffer[this->blocksize];

    std::streamsize bytes_read;
    while (std::cin.read(file_buffer, this->blocksize) || std::cin.gcount() > 0) {
        bytes_read = std::cin.gcount();
        DATA_packet data_packet(++this->blockid, file_buffer, bytes_read);

        n = this->send_and_recv(data_packet.buffer, data_packet.len, 
                                buffer, RQ_PACKETSIZE);
        // check what we got from server
        Opcode op = Utils::get_opcode(buffer, 0);
        switch(op) {
            case Opcode::ACK:
            {
                ACK_packet ack_packet(buffer);
                logger.log_packet(&ack_packet, this->src);
            }
                break;
            case Opcode::ERROR:
            {
                ERROR_packet error_packet(buffer);
                logger.log_packet(&error_packet, this->src, this->dest);
                exit(0);
            }
                break;
            default:
                send_error_packet(Error::ILLEGAL_OPERATION, "CAN'T FIND SUITABLE OPCODE");
                break;
        }
        std::cin.clear();
    }
    // if last read chunk was exactly blocksize
    // send empty packet
    if (bytes_read == this->blocksize) { 
        this->send_empty_data_packet_recv_ack();
    }
}

void Client::setTimeout(int seconds)
{
    this->curr_timeout = seconds;
    this->timeout.tv_sec = seconds;
    this->timeout.tv_usec = 0;

	setsockopt(this->sock, SOL_SOCKET, SO_RCVTIMEO, &this->timeout, sizeof(timeval));
}

void Client::run() {
    // check port limits
    if (this->port < 0 || this->port > 65535) {
        error_exit("wrong port (use 0-65535)");
    }

    memset(&(this->server),0,sizeof(this->server));
    this->server.sin_family = AF_INET;

    if((this->servent = gethostbyname(this->hostname.c_str())) == NULL) {
        error_exit("wrong host");
    }

    memcpy(&(this->server.sin_addr), this->servent->h_addr, this->servent->h_length);
    this->server.sin_port = htons(this->port);
    // create socket
    if((this->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        error_exit("socket creation error");
    }

    // set default timeout
    this->setTimeout(this->default_timeout);
    // set source ip and port
    this->src = Utils::find_src(&(this->server));

    if(this->opcode == Opcode::WRQ){
        this->WRQ();
    } else {
        this->RRQ();
    }
}
