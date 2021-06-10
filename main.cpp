#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>
#include <map>
#include <fstream>
#include <string>
#include <vector>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "SFML/Network.hpp"
#include "Servo.h"
#include "Klient.h"

using namespace std;

/* long long hash_pass(string pas){
    long long p = 997;
    long long mod = 1e9+7;
    if(pas.size() <= 0)
        return -1;
    long long val = pas[0];
    for(unsigned int i=1; i<pas.size(); ++i)
        val = (val*p + pas[i]) % mod;
    return val;
}*/

sf::TcpListener main_listener;

void signal_callback_handler(int signum){
    main_listener.close();
    cout << "Server closed" << endl;
    exit(signum);
}

void* init_server(void* porto){
    Servo* s = new Servo((intptr_t)porto);
    s->wait_for_command();
    return 0;
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
    cout << "Do you want to be: \n0 - Server \n1 - Client \nIf you want to exit, type anything else" << endl;
    cin >> choice;
    if(choice != 0 && choice != 1){
        cout << "Bye!" << endl;
        return 0;
    }
    if(choice == 0){
        int c_port = 20000;
//        cout << "Specify main port: ";
//        cin >> c_port;
	signal(SIGINT, signal_callback_handler);
        sf::TcpSocket new_client;
        int port_modif = 0;
        main_listener.listen(c_port);

        while(port_modif < 100){

            if(main_listener.accept(new_client) != sf::Socket::Done)
                cout << "Nie udalo sie polaczyc" << endl;
            else{
                ++port_modif;
                if(new_client.send(int_to_chars(c_port+port_modif), 10) != sf::Socket::Done) //wysylamy klientowi nowy port, na ktory ma sie polaczyc
                    continue;
                new_client.disconnect();
                //nowy watek
                pthread_t nowy_proces;
                pthread_create(&nowy_proces, NULL, init_server, (void*)(c_port+port_modif));
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

