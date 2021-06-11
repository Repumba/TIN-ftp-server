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
        pthread_exit(0);
    }
    error_handler(wait_for_password());
    create_maskfile();
    this->update_fs();
}

Servo::~Servo(){
    //client.disconnect();
    for(unsigned int i=0; i<pliki.size(); ++i)
        delete pliki[i];
    delete_maskfile();
    pthread_exit(0);
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

string Servo::hash_password(string pas){
    return sha256(pas);
    /*if(pas.size() <= 0)
        return -1;
    long long val = pas[0];
    for(unsigned int i=1; i<pas.size(); ++i)
        val = (val*p + pas[i]) % mod;
    return val;*/
}

void Servo::create_maskfile(){
    //string temp = "echo . > " + (string)maskfile;
    //system(temp.c_str());
}

void Servo::lock_maskfile(){
    while(!check_maskfile_lock()){
        string command = "echo . > " + maskfile + maskfile_lock;
        system(command.c_str());
    }
}

void Servo::unlock_maskfile(){
    string command;
#ifdef _WIN32
        command = "del ";
# else
        command = "rm ";
#endif // _WIN32
    command += maskfile + maskfile_lock;
    system(command.c_str());
}

void Servo::delete_maskfile(){
    string command;
#ifdef _WIN32
        command = "del ";
# else
        command = "rm ";
#endif // _WIN32
    string temp = command + (string)maskfile;
    system(temp.c_str());
}

bool Servo::check_maskfile_lock(){
    ifstream f(maskfile + maskfile_lock);
    return f.good();
}

void Servo::maskfile_append(string filepath){
    char mask = '0';
    string temp = "echo " + filepath + mask + " >> " + (string)maskfile;
    system(temp.c_str());
}

char Servo::check_mask(string filepath){
    lock_maskfile();
    char mask = '0';
    fstream f;

#ifdef _WIN32
    f.open(maskfile, ios_base::in);
#else
    f.open(maskfile, ios::in);
#endif // _WIN32

    string s;
    while(f >> s){
        if (s.substr(0, s.size()-1) == filepath)
            mask = s[s.size()-1];
    }
    f.close();
    unlock_maskfile();

    mask -= '0';
    return mask;
}

void Servo::set_mask(string filepath, char mask){
    lock_maskfile();
    char cur_mask = '0';
    fstream f;

#ifdef _WIN32
    f.open(maskfile, ios_base::in);
#else
    f.open(maskfile, ios::in);
#endif // _WIN32

    string s;
    while(f >> s){
        if (s.substr(0, s.size()-1) == filepath){
            cur_mask = s[s.size()-1];
        }
    }
    cur_mask -= '0';
    cur_mask = cur_mask | mask;
    cur_mask += '0';

    f.close();
    string temp = "echo " + filepath + cur_mask + " >> " + (string)maskfile;
    system(temp.c_str());
    unlock_maskfile();
}


void Servo::unset_mask(string filepath, char mask){
    lock_maskfile();
    char cur_mask = '0';
    fstream f;

#ifdef _WIN32
    f.open(maskfile, ios_base::in);
#else
    f.open(maskfile, ios::in);
#endif // _WIN32

    string s;
    while(getline(f, s)){
        if (s.substr(0, s.size()-2) == filepath){
            cur_mask = s.back();
        }
    }

    cur_mask -= '0';
    cur_mask = cur_mask & ~mask;
    cur_mask += '0';

    f.close();
    string temp = "echo " + filepath + cur_mask + " >> " + (string)maskfile;
    system(temp.c_str());
    unlock_maskfile();
}

void Servo::update_fs(){
    lock_maskfile();
    create_maskfile();
    for(unsigned int i=0; i<pliki.size(); ++i)
        delete pliki[i];
    int files_size = 0;
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
        new_file->isDir = new_file->name.find(".") == string::npos ? true : false;
        maskfile_append(new_file->path + new_file->name);
        pliki.push_back(new_file);
        int len = userName.size();
        if(new_file->path.substr(0,len) == userName){
            struct stat sb;
            stat(make_windows_path(s).c_str(), &sb);
            files_size += sb.st_size;
        }
    }
    f.close();
    system("del temp.txt");
    currentSize = files_size;

#else
    system("ls -1R > temp.txt");

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
        maskfile_append(sciezka + s);
        pliki.push_back(nowy_plik);
        int len = userName.size();
        if(nowy_plik->path.substr(0,len) == userName){
            struct stat sb;
            s = path + s;
            if(stat(s.c_str(), &sb) == -1)
                continue;
            files_size += sb.st_size;
        }
    }
    f.close();
    system("rm temp.txt");
    currentSize = files_size;


#endif // _WIN32
    unlock_maskfile();
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
            this->path = username;
            this->path += "/";
            this->userName = username;
            return 0;
        }else{
            error_handler(2);
        }
    }
    return 2;
}

bool Servo::check_password(string user, string passwd){
    fstream f;
    f.open("passwd.txt", ios::in);
    if(!f.is_open()){
        cout << "Error no passwd file" << endl;
        exit(1);
    }

    string read_user;
    string read_hash;
    string read_salt;
    while(f >> read_user >> read_hash >> read_salt){
        if (user == read_user){
            string hash = sha256(read_salt + passwd);
            if (hash == read_hash){
                f.close();
                return true;
            }
            else{
                break;
            }
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

    if(nazwa_pliku.find("/") != string::npos || nazwa_pliku.find(".") == string::npos){
        if(client.send(int_to_chars(nazwa_pliku.size()), 100) != sf::Socket::Done) //send to client the size of the file
            return 1;
        if(client.send(nazwa_pliku.c_str(), nazwa_pliku.size()) != sf::Socket::Done) //send the file to the client
            return 1;
        return 11;
    }

    fstream pliczek;
    set_mask(path+nazwa_pliku, 2);
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
    pliczek.close();
    unset_mask(path+nazwa_pliku, 2);
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

    if(client.receive(dane, rozmiar_pliku, received) != sf::Socket::Done) //wczytanie, zeby sie nie popsulo
        return 1;
    if(nazwa_pliku.find("/") != string::npos || nazwa_pliku.find(".") == string::npos)
        return 11;
    if(exist_file(nazwa_pliku))
        return 4;
    if(currentSize+rozmiar_pliku > maxSize)
        return 9;

    fstream pliczek;
    set_mask(path+nazwa_pliku, 1);
    pliczek.open(path+nazwa_pliku, fstream::out);
    if(!pliczek.is_open()){
        return -1;
    }
    dane[rozmiar_pliku] = '\0';
    pliczek << dane;
    pliczek.close();
    unset_mask(path+nazwa_pliku, 1);

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
    if(nazwa_pliku.find("/") != string::npos || nazwa_pliku.find(".") == string::npos){
        return 11;
    }
    string abs_path = path + nazwa_pliku;
    char mask = check_mask(abs_path);
    if(mask == 1) return 6;
    if(mask == 2) return 5;

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
    if(nazwa_folderu.find("/") != string::npos || nazwa_folderu.find(".") != string::npos){
        return 11;
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
        if(zaglebienie < 2) //nie da sie bardziej cofnac
            return 7;
        int w = path.size()-1;
        path[w--] = '\0';
        while(w>=0 && path[w] != '/')
            path[w--] = '\0';
        return 0;
    }
    if(nazwa_folderu.find("/") != string::npos || nazwa_folderu.find(".") != string::npos){
        return 11;
    }
    if(!exist_file(nazwa_folderu))
        return 3;
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
    if(nazwa_pliku.find("/") != string::npos || nazwa_pliku.find(".") == string::npos){
        return 11;
    }

    char mask = np[0] - '0';
    if(mask == 0)
        return 0;
    string abs_path = path + nazwa_pliku;
    for(unsigned int i=0; i<pliki.size(); ++i){
        if(pliki[i]->path + pliki[i]->name == abs_path){
            set_mask(abs_path, mask);
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
    if(nazwa_pliku.find("/") != string::npos || nazwa_pliku.find(".") == string::npos){
        return 11;
    }

    char mask = np[0] - '0';
    if(mask == 0)
        return 0;
    string abs_path = path + nazwa_pliku;
    for(unsigned int i=0; i<pliki.size(); ++i){
        if(pliki[i]->path + pliki[i]->name == abs_path){
            unset_mask(abs_path, mask);
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
        pthread_exit(0);
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
    case 9:
        cout << "Tried to exceed memory limit " << client.getRemoteAddress() << " on port: " << client.getLocalPort() << endl;
        break;
    case 10:
        cout << "Undefined behavior " << client.getRemoteAddress() << " on port: " << client.getLocalPort() << endl;
        break;
    case 11:
        cout << "Invalid syntax " << client.getRemoteAddress() << " on port: " << client.getLocalPort() << endl;
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
            listener.close();
            return;
        default:
            kod_bledu = -1;
        }
        //cout << comm[0] << endl;
        error_handler(kod_bledu);
    }
}
