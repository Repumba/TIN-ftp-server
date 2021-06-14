#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SFML/Network.hpp"

#ifndef CLIENT_H
#define CLIENT_H

class Client{
private:
    sf::TcpSocket server;

    int ask_ls();
    int ask_send_file();
    int ask_receive_file();
    int ask_delete_file();
    int ask_make_directory();
    int ask_change_directory();
    int ask_lock_file();
    int ask_unlock_file();
    int login();

    int chars_to_int(char*);
    int send_command(char);
    char* int_to_chars(int);
    char* read_input(std::string, int);

    void error_handler(int);
public:
    Client();
    Client(std::string, int);
    ~Client();
    void wait_for_instruction();
};
#endif // CLIENT_H
