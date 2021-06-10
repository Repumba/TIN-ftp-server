#include <iostream>
#include <map>
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
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
    std::string maskfile = "access.masks";
    std::string maskfile_lock = ".lock";

    long long hash_password(std::string);
    void update_fs();
    void change_mask(std::string, int);
    int wait_for_password();
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

    void create_maskfile();
    void lock_maskfile();
    void unlock_maskfile();
    void delete_maskfile();
    bool check_maskfile_lock();
    void maskfile_append(std::string);
    char check_mask(std::string);
    void set_mask(std::string, char);
    void unset_mask(std::string, char);

    int chars_to_int(char*);
    char* int_to_chars(int);
    std::string make_windows_path(std::string);
    std::string make_linux_path(std::string);

    void error_handler(int);
public:
    Servo(); //tylko do testow - potem wywalic
    Servo(int);
    ~Servo();
    void wait_for_command();
};
#endif // SERVO_H
