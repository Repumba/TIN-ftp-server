#include <string>

struct MyFile{
    char mask = 0; //last bit - writing, previous bit - reading
    bool isDir = false;
    std::string name = "";
    std::string path = "";
};
