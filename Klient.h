#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SFML/Network.hpp"

#ifndef KLIENT_H
#define KLIENT_H

class Klient{
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
    int chars_to_int(char*);
    int send_command(char);
    char* int_to_chars(int);
    char* read_input(std::string, int);
public:
    Klient(); //do testow - potem wywalic
    Klient(std::string, int);
    ~Klient();
    void wait_for_instruction();
};
#endif // KLIENT_H
