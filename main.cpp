#include <iostream>
#include <map>
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SFML/Network.hpp"
#include "Servo.h"
#include "Klient.h"

using namespace std;

void init_server(int porto){
    Servo* s = new Servo(porto);
    s->wait_for_command();
}

char* int_to_chars(int val){
    char* output = new char[10];
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

int main(){
    int choice = -1;
    cout << "Do you want to be: \n0 - Server \n1 - Client \nIf you want to exit type anything else" << endl;
    cin >> choice;
    if(choice != 0 && choice != 1){
        cout << "Bye!" << endl;
        return 0;
    }
    if(choice == 0){
        int c_port = 20000;
//        cout << "Specify main port: ";
//        cin >> c_port;
        sf::TcpListener main_listener;
        sf::TcpSocket new_client;
        int port_modif = 0;
        while(port_modif < 100){
            main_listener.listen(c_port);
            if(main_listener.accept(new_client) != sf::Socket::Done)
                cout << "Nie udalo sie polaczyc" << endl;
            else{
                ++port_modif;
                if(new_client.send(int_to_chars(c_port+port_modif), 10) != sf::Socket::Done) //wysylamy klientowi nowy port, na ktory ma sie polaczyc
                    continue;
                new_client.disconnect();
                //nowy watek
                init_server(c_port+port_modif);
            }
        }
    } else {
        string c_ip;
        int c_port;
        cout << "Specify server IP address: ";
        cin >> c_ip;
        cout << "Specify port: ";
        cin >> c_port;
        Klient* k = new Klient(c_ip, c_port);
        k->wait_for_instruction();
    }
    cout << "Hello World!" << endl;

    return 0;
}

