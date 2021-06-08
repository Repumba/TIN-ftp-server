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

int main(){

    Servo* s = new Servo();
    Klient* k = new Klient();
    cout << "Hello World!" << endl;

    return 0;
}

