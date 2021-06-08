#include <iostream>
#include <map>
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SFML/Network.hpp"
#include "myFile.h"

#ifndef SERVO_H
#define SERVO_H

class Servo{
private:
    std::vector<MyFile*> pliki;
    long long mod = 1e9+7;
    long long p = 997;
    sf::TcpListener listener;
    sf::TcpSocket client;
    std::string path="";

    long long hash_password(std::string);
    void update_fs();
    void change_mask(std::string, int);
    void wait_for_password();
    int send_ls();
    bool check_password(std::string, std::string);
    int send_file();
    int receive_file();
    int delete_file();
    int make_directory();
    int change_directory();
    int lock_file();
    int unlock_file();
    bool exist_file(std::string);

    int chars_to_int(char*);
    char* int_to_chars(int);
    std::string make_windows_path(std::string);
    std::string make_linux_path(std::string);
public:
    Servo(); //tylko do testow - potem wywalic
    Servo(int);
    ~Servo();
    void wait_for_command();
};
#endif // SERVO_H
