#include "Klient.h"
using namespace std;

Klient::Klient(){

}

Klient::Klient(string ip, int portNum){
    if(server.connect(ip, portNum) != sf::Socket::Done)
        error_handler(1);

    char new_port[10];
    size_t received;
    if(server.receive(new_port, 10, received) != sf::Socket::Done)
        error_handler(1);
    server.disconnect();

    if(server.connect(ip, chars_to_int(new_port)) != sf::Socket::Done)
        error_handler(1);

    login();
}
Klient::~Klient(){
    server.disconnect();
}

int Klient::login(){
    for(int i=0; i<3; ++i){
        string username, password;
        cout << "Username: ";
        cin >> username;
        cout << "Password: ";
        cin >> password;
        if(server.send(username.c_str(), 100) != sf::Socket::Done)
            return 1;
        if(server.send(password.c_str(), 100) != sf::Socket::Done)
            return 1;
        char resp[5];
        size_t received;
        if(server.receive(resp, 5, received) != sf::Socket::Done)
            return 1;
        if((int)resp[0] == 0)
            return 0;
        cout << "Authentication failed" << endl;
    }
    exit(0);
}

int Klient::chars_to_int(char* tab){
    string s = tab;
    int ret = 0;
    for(unsigned int i=0; i<s.size(); ++i){
        ret = ret*10 + tab[i]-'0';
    }
    return ret;
}

char* Klient::int_to_chars(int val){
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

int Klient::send_command(char c){
    char comm[5];
    comm[0] = c;
    comm[1] = '\0';
    if(server.send(comm, 5) != sf::Socket::Done){
        return 1;
    }
    return 0;
}

char* Klient::read_input(string info, int rozmiar=100){
    string s;
    cout << info;
    cin >> s;
    char* ret = new char[rozmiar];
    for(unsigned int i=0; i<s.size(); ++i){
        ret[i] = s[i];
    }
    ret[s.size()] = '\0';
    return ret;
}

int Klient::ask_ls(){
    if(send_command('a') != 0)
        return 1;
    char output[1000];
    size_t received;
    if(server.receive(output, 1000, received) != sf::Socket::Done){
        return 1;
    }
    if(output[0] == '\0'){
        cout << "Nothing to show\n";
    } else {
        cout << output;
    }
    return 0;
}

int Klient::ask_send_file(){
    char* nazwa_pliku = read_input("Podaj nazwe pliku na serwerze\n");
    char* lokalny_plik = read_input("Podaj nazwe pliku lokalnego\n");
    if(send_command('b') != 0)
        return 1;
    if(server.send(nazwa_pliku, 100) != sf::Socket::Done)
        return 1;

    char rozmiar_char[100];
    size_t received;
    if(server.receive(rozmiar_char, 100, received) != sf::Socket::Done)
        return 1;
    int rozmiar = chars_to_int(rozmiar_char);

    char plik[rozmiar];
    if(server.receive(plik, rozmiar, received) != sf::Socket::Done)
        return 1;

    fstream f;
    f.open(lokalny_plik, fstream::out);
    f << plik;
    f.close();

    return 0;
}

int Klient::ask_receive_file(){
    char* nazwa_pliku = read_input("Podaj nazwe pliku na serwerze\n");
    char* lokalny_plik = read_input("Podaj nazwe pliku lokalnego\n");
    if(send_command('c') != 0)
        return 1;
    if(server.send(nazwa_pliku, 100) != sf::Socket::Done)
        return 1;
    //calculates the size of the file
    fstream pliczek;
    pliczek.open(lokalny_plik, fstream::in);
    long beg, fin;
    beg = pliczek.tellg();
    pliczek.seekg(0, ios::end);
    fin = pliczek.tellg();
    int size_of_file = fin-beg;
    //moves to the beginning of the file
    if(server.send(int_to_chars(size_of_file), 100) != sf::Socket::Done)
        return 1;
    pliczek.clear();
    pliczek.seekg(0, ios::beg);

    char plik[size_of_file];
    int i=0;
    while(!pliczek.eof()){
        pliczek.get(plik[i++]);
    }
    if(server.send(plik, size_of_file) != sf::Socket::Done)
        return 1;
    return 0;
}

int Klient::ask_delete_file(){
    char* nazwa_pliku = read_input("Podaj nazwe pliku na serwerze\n");
    if(send_command('d') != 0)
        return 1;
    if(server.send(nazwa_pliku, 100) != sf::Socket::Done)
        return 1;
    return 0;
}

int Klient::ask_make_directory(){
    char* nazwa_folderu = read_input("Podaj nazwe nowego folderu na serwerze\n");
    if(send_command('e') != 0)
        return 1;
    if(server.send(nazwa_folderu, 100) != sf::Socket::Done)
        return 1;
    return 0;
}

int Klient::ask_change_directory(){
    char* nowy_folder = read_input("Do jakiego folderu przejsc?\n");
    if(send_command('f') != 0)
        return 1;
    if(server.send(nowy_folder, 100) != sf::Socket::Done)
        return 1;
    return 0;
}

int Klient::ask_lock_file(){
    char* selected_file = read_input("Jaki plik zablokowac?\n");
    char* new_mask = read_input("0 - zablokuj zapis\n1 - zablokuj odczyt\n2 - powrot\n");
    new_mask[1] = '\0';
    if(new_mask[0] == '2')
        return 0;
    if(send_command('g') != 0)
        return 1;
    if(server.send(selected_file, 100) != sf::Socket::Done)
        return 1;
    if(server.send(new_mask, 5) != sf::Socket::Done)
        return 1;
    return 0;
}

int Klient::ask_unlock_file(){
    char* selected_file = read_input("Jaki plik odblokowac?\n");
    char* new_mask = read_input("0 - odblokuj zapis\n1 - odblokuj odczyt\n2 - powrot\n");
    new_mask[1] = '\0';
    if(new_mask[0] == '2')
        return 0;
    if(send_command('h') != 0)
        return 1;
    if(server.send(selected_file, 100) != sf::Socket::Done)
        return 1;
    if(server.send(new_mask, 100) != sf::Socket::Done)
        return 1;
    return 0;
}

void Klient::error_handler(int err_code){
    char server_error_code[5];
    size_t received;
    if(server.receive(server_error_code, 5, received) != sf::Socket::Done)
        err_code = 1;
    else
        if(err_code == 0)
            err_code = (int)server_error_code[0];

    switch(err_code){
    case 1:
        cout << "Connection error with server " << server.getRemoteAddress() << " on port: " << server.getLocalPort() << endl;
        server.disconnect();
        exit(0);
    case 2:
        cout << "Wrong login data " << server.getRemoteAddress() << " on port: " << server.getLocalPort() << endl;
        break;
    case 3:
        cout << "Tried to access not existing file " << server.getRemoteAddress() << " on port: " << server.getLocalPort() << endl;
        break;
    case 4:
        cout << "Tried to modify already existing file " << server.getRemoteAddress() << " on port: " << server.getLocalPort() << endl;
        break;
    case 5:
        cout << "Tried to access read-blocked file " << server.getRemoteAddress() << " on port: " << server.getLocalPort() << endl;
        break;
    case 6:
        cout << "Tried to access write-blocked file " << server.getRemoteAddress() << " on port: " << server.getLocalPort() << endl;
        break;
    case 7:
        cout << "Tried to overcome restrictions " << server.getRemoteAddress() << " on port: " << server.getLocalPort() << endl;
        break;
    case 8:
        cout << "Tried to open file as a directory " << server.getRemoteAddress() << " on port: " << server.getLocalPort() << endl;
        break;
    case 9:
        cout << "Tried to exceed memory limit " << server.getRemoteAddress() << " on port: " << server.getLocalPort() << endl;
        break;
    case 10:
        cout << "Undefined behavior " << server.getRemoteAddress() << " on port: " << server.getLocalPort() << endl;
        break;
    }
    case 11:
        cout << "Invalid syntax " << server.getRemoteAddress() << " on port: " << server.getLocalPort() << endl;
        break;
    }
    return;
}

void Klient::wait_for_instruction(){
    char wyb;
    int kod_bledu;
    while(true){
        cout << "> ";
        cin >> wyb;

        switch(wyb){
        case 'a':
            kod_bledu = ask_ls();
            break;
        case 'b':
            kod_bledu = ask_send_file();
            break;
        case 'c':
            kod_bledu = ask_receive_file();
            break;
        case 'd':
            kod_bledu = ask_delete_file();
            break;
        case 'e':
            kod_bledu = ask_make_directory();
            break;
        case 'f':
            kod_bledu = ask_change_directory();
            break;
        case 'g':
            kod_bledu = ask_lock_file();
            break;
        case 'h':
            kod_bledu = ask_unlock_file();
            break;
        case 'x':
            send_command('x');
            server.disconnect();
            cout << "Bye!" << endl;
            exit(0);
        default:
            kod_bledu = 0;
            cout << "Dostepne polecenia:\n";
            cout << "a - wyswietl zawartosc folderu\n";
            cout << "b - pobranie pliku z serwera\n";
            cout << "c - wyslanie pliku na serwer\n";
            cout << "d - usuniecie pliku\n";
            cout << "e - utworzenie folderu\n";
            cout << "f - zmienienie aktualnego folderu\n";
            cout << "g - zablokowanie pliku\n";
            cout << "h - odblokowanie pliku\n";
            cout << "x - zakoncz polaczenie" << endl; //tutaj endl, aby wymusic oproznienie bufora
            continue;
        }

        error_handler(kod_bledu);
    }
}
