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

int main(int argc, char* argv[]){

    if(argc == 1){
        Servo* s = new Servo(20000);
        while(true){
            s->wait_for_command();
        }
    } else {
        Klient* k = new Klient("192.168.1.240", 20000);
        k->wait_for_instruction();
    }
    cout << "Hello World!" << endl;

    return 0;
}

