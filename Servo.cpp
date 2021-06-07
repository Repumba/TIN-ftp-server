#include "Servo.h"
using namespace std;

Servo::Servo(int portNum){
    listener.listen(portNum);
    if(listener.accept(client) != sf::Socket::Done)
        cout << "Error" << endl;
    this->update_fs();
}

Servo::~Servo(){
    client.disconnect();
}

int Servo::chars_to_int(char* tab, int len){
    int ret = 0;
    for(int i=0; i<len; ++i){
        ret = ret*10 + tab[i]-'0';
    }
    return ret;
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
    //system("ls -1Rp > temp.txt");
    system("dir > temp.txt");
    fstream f;
    f.open("temp.txt", ios::in);
    string sciezka, plik, s;
    while(f >> s){
        if(s[0] == '.'){
            sciezka = s;
            sciezka[sciezka.size()-1] = '/';
            continue;
        }
        if(s[s.size()-1] == '/'){ //folder
            MyFile* nowy_plik = new MyFile;
            nowy_plik->isDir = true;
            nowy_plik->name = s;
            nowy_plik->path = sciezka;
            pliki.push_back(nowy_plik);
        } else{ //not folder
            MyFile* nowy_plik = new MyFile;
            nowy_plik->name = s;
            nowy_plik->path = sciezka;
            pliki.push_back(nowy_plik);
        }
    }
    f.close();
    //system("rm temp.txt");
    system("del temp.txt");
}

void Servo::wait_for_password(){
//    string username, password;
    char username[100];
    char password[100];
    size_t received;
    if(client.receive(username, 100, received) != sf::Socket::Done)
        cout << "Error";
    if(client.receive(password, 100, received) != sf::Socket::Done)
        cout << "Error";

    if(check_password(username, password)){
        path = username;
        path += "/";
    }else{
        client.send("Wrong password", 32);
    }
}

bool Servo::check_password(string u, string p){
    long long h = hash_password(p);
    fstream f;
    f.open("passwd.txt", ios::in);
    if(!f.is_open()){
        cout << "Error no passwd file" << endl;
        exit(-1);
//        throw exception ("No passwd detected");
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

int Servo::send_file(){
    char np[100]; //nazwa pliku
    size_t received;
    client.receive(np, 100, received);
    string nazwa_pliku = np;
    fstream pliczek;
    pliczek.open(path + nazwa_pliku, ios_base::in);
    if(!pliczek.is_open()){
        cout << "Error" << endl;
        return -1;
    }
    //calculates the size of the file
    long beg, fin;
    beg = pliczek.tellg();
    pliczek.seekg(0, ios::end);
    fin = pliczek.tellg();
    int size_of_file = beg-fin;
    //moves to the beginning of the file
    pliczek.clear();
    pliczek.seekg(0, ios::beg);
    char tab[size_of_file];
    int w=0;
    while(!pliczek.eof()){
        pliczek.get(tab[w++]);
    }
    if(client.send(tab, size_of_file) != sf::Socket::Done)
        cout << "Error" << endl;

    return 0;
}

int Servo::send_ls(){
    char ls[1001];
    int w=0;
    for(unsigned int i=0; i<pliki.size() && w<1000; ++i){
        if(pliki[i]->path == path){
            for(unsigned int j=0; j<pliki[i]->name.size() && w<1000; ++j){
                ls[w++] = pliki[i]->name[j];
            }
        }
        ls[w++] = '\n';
    }
    ls[w++] = '\0';
    if(client.send(ls, w) != sf::Socket::Done)
        cout << "Error" << endl;
}

bool Servo::exist_file(string s){
    for(unsigned int i=0; i<pliki.size(); ++i)
        if(pliki[i]->path + pliki[i]->name == path + s)
            return true;

    return false;
}

int Servo::receive_file(){
    char np[100];
    size_t received;
    client.receive(np, 100, received); //nazwa pliku
    string nazwa_pliku = np;
    if(exist_file(nazwa_pliku)){
        return -1;
    }
    client.receive(np, 100, received); //rozmiar pliku
    int rozmiar_pliku = chars_to_int(np, received);
    fstream pliczek;
    pliczek.open(path + nazwa_pliku, ios_base::out);
    if(!pliczek.is_open()){
        cout << "Error" << endl;
        return -1;
    }
    char dane[rozmiar_pliku];
    client.receive(dane, rozmiar_pliku, received);
    pliczek << dane;
    pliczek.close();
    return 0;
}

int Servo::delete_file(){
    char np[100];
    size_t received;
    client.receive(np, 100, received); //nazwa pliku
    string nazwa_pliku = np;
    if(!exist_file(nazwa_pliku)){
        return -1;
    }
    string abs_path = path + nazwa_pliku;
    for(int i=0; i<pliki.size(); ++i){
        if(pliki[i]->path + pliki[i]->name == abs_path){
            if(pliki[i]->isDir){
                //TODO
                return -1;
            }
            abs_path = "rm " + abs_path;
            system(abs_path.c_str());
            delete pliki[i];
            pliki[i] = pliki[pliki.size()-1];
            pliki.pop_back();
            return 0;
        }
    }
    return -1;
}

int Servo::make_directory(){
    char np[100];
    size_t received;
    client.receive(np, 100, received); //nazwa nowego folderu
    string nazwa_folderu = np;
    if(exist_file(nazwa_folderu)){
        return -1;
    }
    MyFile* new_dir = new MyFile;
    new_dir->isDir = true;
    new_dir->name = nazwa_folderu;
    new_dir->path = path;
    pliki.push_back(new_dir);
    return 0;
}

int Servo::change_directory(){
    char np[100];
    size_t received;
    client.receive(np, 100, received); //nazwa nowego folderu
    string nazwa_folderu = np;
    if(nazwa_folderu == ".."){
        int zaglebienie=0;
        for(unsigned int i=0; i<path.size(); ++i)
            if(path[i] == '/')
                ++zaglebienie;
        if(zaglebienie < 2) //nie da sie bardziej cofnac
            return -1;
        int w = path.size()-1;
        path[w--] = '\0';
        while(path[w] != '/')
            path[w--] = '\0';
        return 0;
    }
    string abs_path = path + nazwa_folderu;
    for(int i=0; i<pliki.size(); ++i){
        if(pliki[i]->path + pliki[i]->name == abs_path){
            if(!pliki[i]->isDir){
                return -1;
            }
            path += "/" + nazwa_folderu;
            return 0;
        }
    }
}

int Servo::lock_file(){
    char np[100]; //nazwa pliku
    size_t received;
    client.receive(np, 100, received);
    string nazwa_pliku = np;
    if(!exist_file(nazwa_pliku)){
        return -1;
    }
    client.receive(np, 5, received);
    char mask = np[0];
    string abs_path = path + nazwa_pliku;
    for(int i=0; i<pliki.size(); ++i){
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
    client.receive(np, 100, received);
    string nazwa_pliku = np;
    if(!exist_file(nazwa_pliku)){
        return -1;
    }
    client.receive(np, 5, received);
    char mask = np[0];
    mask = ~mask;
    string abs_path = path + nazwa_pliku;
    for(int i=0; i<pliki.size(); ++i){
        if(pliki[i]->path + pliki[i]->name == abs_path){
            pliki[i]->mask = pliki[i]->mask & mask;
            return 0;
        }
    }
    return -1;
}

void Servo::wait_for_command(){
    char comm[5];
    size_t received;
    if(client.receive(comm, 5, received) != sf::Socket::Done)
        cout << "Error" << endl;

    int kod_bledu = 0;
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
    }
    if(kod_bledu != 0){
        cout << "Something went wrong" << endl;
    }
}
