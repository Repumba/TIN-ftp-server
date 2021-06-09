#include "Servo.h"
using namespace std;

#ifdef _WIN32
#include <string>
#include <windows.h>
#include <iostream>

string ExePath() {
    char buffer[MAX_PATH] = { 0 };
    GetModuleFileName( NULL, buffer, MAX_PATH );
    string s = buffer;
    int pos = s.find_last_of("\\")+1;
    return s.substr(0, pos);
}
#endif // _WIN32


Servo::Servo(){

}

Servo::Servo(int portNum){
    listener.listen(portNum);
    if(listener.accept(client) != sf::Socket::Done){
        cout << "Nie udalo sie polaczyc" << endl;
        exit(1);
    }
    error_handler(wait_for_password());
    this->update_fs();
}

Servo::~Servo(){
    client.disconnect();
}

string Servo::make_windows_path(string s){
    for(unsigned int i=0; i<s.size(); ++i)
        if(s[i] == '/')
            s[i] = '\\';
    return s;
}

string Servo::make_linux_path(string s){
    for(unsigned int i=0; i<s.size(); ++i)
        if(s[i] == '\\')
            s[i] = '/';
    return s;
}

int Servo::chars_to_int(char* tab){
    string s = tab;
    int ret = 0;
    for(unsigned int i=0; i<s.size(); ++i){
        ret = ret*10 + tab[i]-'0';
    }
    return ret;
}

char* Servo::int_to_chars(int val){
    char* output = new char[30];
    int i = 30;
    while(val > 0){
        output[--i] = val%10 + '0';
        val /= 10;
    }
    int w = 0;
    while(w+i < 30){
        output[w] = output[i+w];
        ++w;
    }
    output[w] = '\0';
    return output;
}

long long Servo::hash_password(string pas){
    if(pas.size() <= 0)
        return -1;
    long long val = pas[0];
    for(unsigned int i=1; i<pas.size(); ++i)
        val = (val*p + pas[i]) % mod;
    return val;
}

void Servo::update_fs(){
    for(unsigned int i=0; i<pliki.size(); ++i)
        delete pliki[i];
    pliki.clear();

#ifdef _WIN32
    string system_path = ExePath();
    system("dir /S /B > temp.txt");
    fstream f;
    f.open("temp.txt", ios_base::in);
    string s;
    while(f >> s){
        if(s.size() <= system_path.size())
            continue;
        s = s.substr(system_path.size(), MAX_PATH);
        s = make_linux_path(s);
        int pos = s.find_last_of("/")+1;
        MyFile* new_file = new MyFile;
        new_file->path = s.substr(0,pos);
        new_file->name = s.substr(pos, s.size());
        new_file->mask = 0;
        new_file->isDir = new_file->name.find(".") == string::npos ? true : false;
        pliki.push_back(new_file);
    }
    f.close();
    system("del temp.txt");
    return;
#else
    system("ls -1Rp > temp.txt");

    fstream f;
    f.open("temp.txt", ios::in);
    string sciezka, plik, s;
    while(f >> s){
        if(s[0] == '.'){
            if(s.size() == 2){
                sciezka = "";
            } else {
                sciezka = s.substr(2, s.size()-3);
                sciezka += "/";
            }
            continue;
        }
        MyFile* nowy_plik = new MyFile;
        nowy_plik->name = s;
        nowy_plik->path = sciezka;
        nowy_plik->isDir = s.find(".") == string::npos ? true : false;
        pliki.push_back(nowy_plik);
    }
    f.close();
    system("rm temp.txt");
    return;
#endif // _WIN32
}

int Servo::wait_for_password(){
    for(int i=0; i<3; ++i){
        char username[100];
        char password[100];
        size_t received;
        if(client.receive(username, 100, received) != sf::Socket::Done){
            return 1;
        }
        if(client.receive(password, 100, received) != sf::Socket::Done){
            return 1;
        }

        if(check_password(username, password)){
            path = username;
            path += "/";
            return 0;
        }else{
            error_handler(2);
        }
    }
    return 2;
}

bool Servo::check_password(string u, string p){
    long long h = hash_password(p);
    fstream f;
    f.open("passwd.txt", ios::in);
    if(!f.is_open()){
        cout << "Error no passwd file" << endl;
        exit(1);
    }
    string wu;
    long long wh;
    while(f >> wu >> wh){
        if(!wu.compare(u))
            if(h == wh){
                f.close();
                return true;
            }
    }
    f.close();
    return false;
}

int Servo::send_ls(){
    char ls[1000];
    int w=0;
    for(unsigned int i=0; i<pliki.size() && w<999; ++i){
        if(pliki[i]->path == path){
            for(unsigned int j=0; j<pliki[i]->name.size() && w<999; ++j){
                ls[w++] = pliki[i]->name[j];
            }
            ls[w++] = '\n';
        }
    }
    ls[w++] = '\0';
    if(client.send(ls, 1000) != sf::Socket::Done)
        return 1;
    return 0;
}

bool Servo::exist_file(string s){
    for(unsigned int i=0; i<pliki.size(); ++i)
        if(pliki[i]->path + pliki[i]->name == path + s){
            return true;
        }

    return false;
}

int Servo::send_file(){
    char np[100]; //nazwa pliku
    size_t received;
    if(client.receive(np, 100, received) != sf::Socket::Done)
        return 1;
    string nazwa_pliku = np;
    fstream pliczek;
    pliczek.open(path+nazwa_pliku, fstream::in);
    if(!pliczek.is_open()){
        return -1;
    }
    //calculates the size of the file
    long beg, fin;
    beg = pliczek.tellg();
    pliczek.seekg(0, ios::end);
    fin = pliczek.tellg();
    int size_of_file = fin-beg;
    //moves to the beginning of the file
    pliczek.clear();
    pliczek.seekg(0, ios::beg);

    char tab[size_of_file+1];
    int w=0;
    while(!pliczek.eof()){
        pliczek.get(tab[w++]);
    }
    tab[size_of_file] = '\0';

    if(client.send(int_to_chars(size_of_file), 100) != sf::Socket::Done) //send to client the size of the file
        return 1;
    if(client.send(tab, size_of_file) != sf::Socket::Done) //send the file to the client
        return 1;

    return 0;
}

int Servo::receive_file(){
    char np[100];
    size_t received;
    if(client.receive(np, 100, received) != sf::Socket::Done) //nazwa pliku
        return 1;
    string nazwa_pliku = np;
    if(client.receive(np, 100, received) != sf::Socket::Done) //rozmiar pliku
        return 1;

    int rozmiar_pliku = chars_to_int(np);
    char dane[rozmiar_pliku+1];

    if(exist_file(nazwa_pliku)){
        if(client.receive(dane, rozmiar_pliku, received) != sf::Socket::Done) //wczytanie, zeby sie nie popsulo
            return 1;
        return 4;
    }

    fstream pliczek;
    pliczek.open(path+nazwa_pliku, fstream::out);
    if(!pliczek.is_open()){
        return -1;
    }
    if(client.receive(dane, rozmiar_pliku, received) != sf::Socket::Done)
        return 1;
    dane[rozmiar_pliku] = '\0';
    pliczek << dane;
    cout << dane << endl;
    pliczek.close();

    this->update_fs();
    return 0;
}

int Servo::delete_file(){
    char np[100];
    size_t received;
    if(client.receive(np, 100, received) != sf::Socket::Done) //nazwa pliku
        return 1;
    string nazwa_pliku = np;
    if(!exist_file(nazwa_pliku)){
        return 3;
    }
    string abs_path = path + nazwa_pliku;
    for(unsigned int i=0; i<pliki.size(); ++i){
        if(pliki[i]->path + pliki[i]->name == abs_path){
#ifdef _WIN32
            if(pliki[i]->isDir){
                abs_path = "rmdir /Q/S " + make_windows_path(abs_path);
            } else {
                abs_path = "del " + make_windows_path(abs_path);
            }
            system(abs_path.c_str());
#else
            if(pliki[i]->isDir){
                abs_path = "rm -r " + abs_path;
            } else {
                abs_path = "rm " + abs_path;
            }
            system(abs_path.c_str());
#endif
            this->update_fs();
            return 0;
        }
    }
    return -1;
}

int Servo::make_directory(){
    char np[100];
    size_t received;
    if(client.receive(np, 100, received) != sf::Socket::Done) //nazwa nowego folderu
        return 1;
    string nazwa_folderu = np;
    if(exist_file(nazwa_folderu)){
        return 4;
    }
    string abs_path = "mkdir " + path + nazwa_folderu;
#ifdef _WIN32
    system(make_windows_path(abs_path).c_str());
#else
    system(abs_path.c_str());
#endif // _WIN32
    this->update_fs();
    return 0;
}

int Servo::change_directory(){
    char np[100];
    size_t received;
    if(client.receive(np, 100, received) != sf::Socket::Done) //nazwa nowego folderu
        return 1;
    string nazwa_folderu = np;
    if(nazwa_folderu == ".."){
        int zaglebienie=0;
        for(unsigned int i=0; i<path.size(); ++i)
            if(path[i] == '/')
                ++zaglebienie;
        if(zaglebienie < 1) //nie da sie bardziej cofnac
            return 7;
        int w = path.size()-1;
        path[w--] = '\0';
        while(w>=0 && path[w] != '/')
            path[w--] = '\0';
        return 0;
    }
    string abs_path = path + nazwa_folderu;
    for(unsigned int i=0; i<pliki.size(); ++i){
        if(pliki[i]->path + pliki[i]->name == abs_path){
            if(!pliki[i]->isDir){
                return 8;
            }
            path += nazwa_folderu + "/";
            return 0;
        }
    }
    return -1;
}

int Servo::lock_file(){
    char np[100]; //nazwa pliku
    size_t received;
    if(client.receive(np, 100, received) != sf::Socket::Done)
        return 1;
    string nazwa_pliku = np;
    if(client.receive(np, 5, received) != sf::Socket::Done)
        return 1;

    if(!exist_file(nazwa_pliku)){
        return 3;
    }

    char mask = np[0];
    string abs_path = path + nazwa_pliku;
    for(unsigned int i=0; i<pliki.size(); ++i){
        if(pliki[i]->path + pliki[i]->name == abs_path){
            pliki[i]->mask = pliki[i]->mask | mask;
            return 0;
        }
    }
    return -1;
}

int Servo::unlock_file(){
    char np[100]; //nazwa pliku
    size_t received;
    if(client.receive(np, 100, received) != sf::Socket::Done)
        return 1;
    string nazwa_pliku = np;
    if(client.receive(np, 5, received) != sf::Socket::Done)
        return 1;

    if(!exist_file(nazwa_pliku)){
        return 3;
    }

    char mask = np[0];
    mask = ~mask;
    string abs_path = path + nazwa_pliku;
    for(unsigned int i=0; i<pliki.size(); ++i){
        if(pliki[i]->path + pliki[i]->name == abs_path){
            pliki[i]->mask = pliki[i]->mask & mask;
            return 0;
        }
    }
    return -1;
}

void Servo::error_handler(int err_code){
    if(err_code == -1)
        err_code = 10;
    char code_for_client[5];
    code_for_client[0] = (char)err_code;
    if(client.send(code_for_client, 5) != sf::Socket::Done)
        err_code = 1;

    switch(err_code){
    case 1:
        cout << "Connection error with client " << client.getRemoteAddress() << " on port: " << client.getLocalPort() << endl;
        client.disconnect();
        exit(1);
    case 2:
        cout << "Wrong login data " << client.getRemoteAddress() << " on port: " << client.getLocalPort() << endl;
        break;
    case 3:
        cout << "Tried to access not existing file " << client.getRemoteAddress() << " on port: " << client.getLocalPort() << endl;
        break;
    case 4:
        cout << "Tried to modify already existing file " << client.getRemoteAddress() << " on port: " << client.getLocalPort() << endl;
        break;
    case 5:
        cout << "Tried to access read-blocked file " << client.getRemoteAddress() << " on port: " << client.getLocalPort() << endl;
        break;
    case 6:
        cout << "Tried to access write-blocked file " << client.getRemoteAddress() << " on port: " << client.getLocalPort() << endl;
        break;
    case 7:
        cout << "Tried to overcome restrictions " << client.getRemoteAddress() << " on port: " << client.getLocalPort() << endl;
        break;
    case 8:
        cout << "Tried to open file as a directory " << client.getRemoteAddress() << " on port: " << client.getLocalPort() << endl;
        break;
    case 10:
        cout << "Undefined behavior " << client.getRemoteAddress() << " on port: " << client.getLocalPort() << endl;
        break;
    }
    return;
}

void Servo::wait_for_command(){
    char comm[5];
    int kod_bledu = 0;
    size_t received;

    while(true){
        if(client.receive(comm, 5, received) != sf::Socket::Done)
            error_handler(1);

        switch(comm[0]){
        case 'a':
            kod_bledu = send_ls();
            break;
        case 'b':
            kod_bledu = send_file();
            break;
        case 'c':
            kod_bledu = receive_file();
            break;
        case 'd':
            kod_bledu = delete_file();
            break;
        case 'e':
            kod_bledu = make_directory();
            break;
        case 'f':
            kod_bledu = change_directory();
            break;
        case 'g':
            kod_bledu = lock_file();
            break;
        case 'h':
            kod_bledu = unlock_file();
            break;
        case 'x':
            client.disconnect();
            exit(0);
        default:
            kod_bledu = -1;
        }
        //cout << comm[0] << endl;
        error_handler(kod_bledu);
    }
}
