#ifndef __ERROR_EXIT_HPP__
#define __ERROR_EXIT_HPP__
#include <iostream>

/**
 * @brief print useage of client
 * 
 */
void print_usage();

/**
 * @brief print usage of server
 * 
 */
void print_usage_server();

/**
 * @brief exit with error message
 * 
 * @param msg error message
 */
void error_exit(std::string msg);


#endif // __ERROR_EXIT_HPP__