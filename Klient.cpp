#include "Klient.h"
using namespace std;

Klient::Klient(int portNum){
    listener.listen(portNum);
    if(listener.accept(server) != sf::Socket::Done)
        cout << "Error" << endl;
}
Klient::~Klient(){
    server.disconnect();
}

int Klient::chars_to_int(char* tab){
    string s = tab;
    int ret = 0;
    for(int i=0; i<s.size(); ++i){
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
        return -1;
    }
    return 0;
}

char* Klient::read_input(string info){
    string s;
    cout << info;
    cin >> s;
    char* ret = new char[s.size()];
    for(int i=0; i<s.size(); ++i){
        ret[i] = s[i];
    }
    return ret;
}

int Klient::ask_ls(){
    if(send_command('a') != 0)
        return -1;
    char output[1000];
    size_t received;
    if(server.receive(output, 1000, received) != sf::Socket::Done){
        return -1;
    }
    if(received > 0){
        string s = output;
        cout << s;
    } else {
        cout << "Nothing to show\n";
    }
    return 0;
}

int Klient::ask_send_file(){
    char* nazwa_pliku = read_input("Podaj nazwe pliku na serwerze\n");
    char* lokalny_plik = read_input("Podaj nazwe pliku lokalnego\n");
    if(send_command('b') != 0)
        return -1;
    server.send(nazwa_pliku, 100);

    char rozmiar_char[30];
    size_t received;
    server.receive(rozmiar_char, 30, received);
    int rozmiar = chars_to_int(rozmiar_char);

    char plik[rozmiar];
    string p = plik;
    server.receive(plik, rozmiar, received);

    fstream f;
    f.open(lokalny_plik, ios_base::out);
    //for(int i=0; i<rozmiar; ++i)
    f << p;
    f.close();

    return 0;
}

int Klient::ask_receive_file(){
    char* nazwa_pliku = read_input("Podaj nazwe pliku na serwerze\n");
    char* lokalny_plik = read_input("Podaj nazwe pliku lokalnego\n");
    if(send_command('c') != 0)
        return -1;
    server.send(nazwa_pliku, 100);
    //calculates the size of the file
    fstream pliczek;
    pliczek.open(lokalny_plik);
    long beg, fin;
    beg = pliczek.tellg();
    pliczek.seekg(0, ios::end);
    fin = pliczek.tellg();
    int size_of_file = beg-fin;
    //moves to the beginning of the file
    pliczek.clear();
    pliczek.seekg(0, ios::beg);

    char plik[size_of_file];
    int i=0;
    while(!pliczek.eof())
        pliczek.get(plik[i++]);

    server.send(plik, size_of_file);
    return 0;
}

int Klient::ask_delete_file(){
    char* nazwa_pliku = read_input("Podaj nazwe pliku na serwerze\n");
    if(send_command('d') != 0)
        return -1;
    server.send(nazwa_pliku, 100);
    return 0;
}

int Klient::ask_make_directory(){
    char* nazwa_folderu = read_input("Podaj nazwe nowego folderu na serwerze\n");
    if(send_command('e') != 0)
        return -1;
    server.send(nazwa_folderu, 100);
    return 0;
}

int Klient::ask_change_directory(){
    char* nowy_folder = read_input("Do jakiego folderu przejsc?\n");
    if(send_command('f') != 0)
        return -1;
    server.send(nowy_folder, 100);
}

int Klient::ask_lock_file(){
    char* selected_file = read_input("Jaki plik zablokowac?\n");
    char* new_mask = read_input("0 - zablokuj zapis\n1 - zablokuj odczyt\n2 - powrot\n");
    new_mask[1] = '\0';
    if(new_mask[0] == '2')
        return 0;
    if(send_command('g') != 0)
        return -1;
    server.send(selected_file, 100);
    server.send(new_mask, 5);
    return 0;
}

int Klient::ask_unlock_file(){
    char* selected_file = read_input("Jaki plik odblokowac?\n");
    char* new_mask = read_input("0 - odblokuj zapis\n1 - odblokuj odczyt\n2 - powrot\n");
    new_mask[1] = '\0';
    if(new_mask[0] == '2')
        return 0;
    if(send_command('h') != 0)
        return -1;
    server.send(selected_file, 100);
    server.send(new_mask, 5);
    return 0;
}
