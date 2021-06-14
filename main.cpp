#include <iostream>
#include <map>
#include <fstream>
#include <string>
#include <vector>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>
#include "SFML/Network.hpp"
#include "Servo.h"
#include "Client.h"

using namespace std;

sf::TcpListener mainListener;

void signal_callback_handler(int signum){
    mainListener.close();
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
    while(choice < 0 || choice > 2){
        cout << "Do you want to be: \n0 - Server \n1 - Client \n2 - exit" << endl;
        cin >> choice;
        if(choice == 2){
            cout << "Bye!" << endl;
            return 0;
        }
    }
    if(choice == 0){
        int connectPort = 20000;
        signal(SIGINT, signal_callback_handler);
        sf::TcpSocket newClient;
        int portModif = 0;
        mainListener.listen(connectPort);

        while(portModif < 100){

            if(mainListener.accept(newClient) != sf::Socket::Done)
                cout << "Could not connect" << endl;
            else{
                ++portModif;

                //send new port number for further communication
                if(newClient.send(int_to_chars(connectPort+portModif), 10) != sf::Socket::Done)
                    continue;
                newClient.disconnect();

                //new thread
                pthread_t newProccess;
                pthread_create(&newProccess, NULL, init_server, (void*)(connectPort+portModif));
            }
        }
    } else {
        string connectIP;
        int connectPort;
        cout << "Specify server IP address: ";
        cin >> connectIP;
        cout << "Specify port: ";
        cin >> connectPort;
        Client* client = new Client(connectIP, connectPort);
        client->wait_for_instruction();
    }

    return 0;
}

