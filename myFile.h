#ifndef MY_FILE_H
#define MY_FILE_H

#include <string>

struct MyFile{
    //char mask = 0; //last bit - writing, previous bit - reading
    bool isDir = false;
    std::string name = "";
    std::string path = "";
};
#endif // MY_FILE_H
