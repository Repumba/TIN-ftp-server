#include "Client.h"
using namespace std;

#ifdef _WIN32
#include <conio.h>
string getpass(bool showAsterisk=true){
    //get password

    const char BACKSPACE=8;
    const char RETURN=13;

    string password;
    unsigned char ch=0;

    while((ch=getch())!=RETURN){
        if(ch==BACKSPACE){
            if(password.length()!=0){
                if(showAsterisk)
                    cout <<"\b \b";
                password.resize(password.length()-1);
            }
        } else if(ch==0 || ch==224){ // handle escape sequences
            getch(); // ignore non printable chars
            continue;
        } else {
             password+=ch;
             if(showAsterisk)
                 cout <<'*';
         }
    }
    cout <<endl;
    return password;
}
#else

#include <termios.h>
#include <unistd.h>
#include <stdio.h>

int getch() {
    //get char, but fitted for out program

    int ch;
    struct termios t_old, t_new;

    tcgetattr(STDIN_FILENO, &t_old);
    t_new = t_old;
    t_new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t_new);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &t_old);
    return ch;
}

string getpass(bool showAsterisk=true) {
    //get password

    const char BACKSPACE=127;
    const char RETURN=10;

    string password;
    unsigned char ch=0;

    while((ch=getch())!=RETURN){
        if(ch==BACKSPACE){
            if(password.length()!=0){
                 if(showAsterisk)
                    cout <<"\b \b";
                 password.resize(password.length()-1);
            }
        } else {
            password+=ch;
            if(showAsterisk)
                cout <<'*';
        }
    }
    cout <<endl;
    return password;
}

#endif // _WIN32

Client::Client(){

}

Client::Client(string ip, int portNumber){
    //Create the Client and connect it to the Server

    if(server.connect(ip, portNumber) != sf::Socket::Done)
        error_handler(1);

    //receive new port number for further communication
    char newPort[10];
    size_t received;
    if(server.receive(newPort, 10, received) != sf::Socket::Done)
        error_handler(1);
    server.disconnect();

    if(server.connect(ip, chars_to_int(newPort)) != sf::Socket::Done)
        error_handler(1);

    login();
}

Client::~Client(){
    server.disconnect();
}

int Client::login(){
    //try to authenticate 3 times, if failed, exit program

    for(int i=0; i<3; ++i){
        string user, pass;
        cout << "Username: ";
        cin >> user;
        cout << "Password: ";
        pass = getpass();
        if(server.send(user.c_str(), 100) != sf::Socket::Done)
            return 1;
        if(server.send(pass.c_str(), 100) != sf::Socket::Done)
            return 1;
        char resp[5]; //response
        size_t received;
        if(server.receive(resp, 5, received) != sf::Socket::Done)
            return 1;
        if((int)resp[0] == 0)
            return 0;
        cout << "Authentication failed" << endl;
    }
    exit(0);
}

int Client::chars_to_int(char* arr){
    //convert a number stored as an array of chars into int value
    //arr - array

    string s = arr;
    int ret = 0;
    for(unsigned int i=0; i<s.size(); ++i){
        ret = ret*10 + arr[i]-'0';
    }
    return ret;
}

char* Client::int_to_chars(int val){
    //convert int value into an array, where subsequent fields are subsequents digits from int

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

int Client::send_command(char c){
    //sends single char to the server, each char represent a specific action

    char comm[5];
    comm[0] = c;
    comm[1] = '\0';
    if(server.send(comm, 5) != sf::Socket::Done){
        return 1;
    }
    return 0;
}

char* Client::read_input(string info, int rozmiar=100){
    //reads string from standard input and converts it to an array of chars

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

int Client::ask_ls(){
    //asks server to send content of the current folder and displays it on the standard output

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

int Client::ask_send_file(){
    //asks server to send file and saves it on a local computer

    char* fileName = read_input("Specify name of a file on the server\n");
    char* localFile = read_input("Specify name of a local file\n");
    if(send_command('b') != 0)
        return 1;
    if(server.send(fileName, 100) != sf::Socket::Done)
        return 1;

    char fileSizeChar[100];
    size_t received;
    if(server.receive(fileSizeChar, 100, received) != sf::Socket::Done)
        return 1;
    int fileSize = chars_to_int(fileSizeChar);

    char fileContent[fileSize];
    if(server.receive(fileContent, fileSize, received) != sf::Socket::Done)
        return 1;

    fstream f;
    f.open(localFile, fstream::out);
    f << fileContent;
    f.close();

    return 0;
}

int Client::ask_receive_file(){
    //asks server to receive file send from local computer

    char* fileName = read_input("Specify name of a file on the server\n");
    char* localFile = read_input("Specify name of a local file\n");

    if(send_command('c') != 0)
        return 1;
    if(server.send(fileName, 100) != sf::Socket::Done)
        return 1;
    //calculates the size of the file
    fstream f;
    f.open(localFile, fstream::in);
    long beg, fin;
    beg = f.tellg();
    f.seekg(0, ios::end);
    fin = f.tellg();
    int fileSize = fin-beg;

    if(server.send(int_to_chars(fileSize), 100) != sf::Socket::Done)
        return 1;

    //moves to the beginning of the file
    f.clear();
    f.seekg(0, ios::beg);

    char fileContent[fileSize];
    int i=0;
    while(!f.eof()){
        f.get(fileContent[i++]);
    }
    if(server.send(fileContent, fileSize) != sf::Socket::Done)
        return 1;
    return 0;
}

int Client::ask_delete_file(){
    //ask server to delete a specified file

    char* fileName = read_input("Name a file to delete\n");
    if(send_command('d') != 0)
        return 1;
    if(server.send(fileName, 100) != sf::Socket::Done)
        return 1;
    return 0;
}

int Client::ask_make_directory(){
    //ask server to create a new directory

    char* folderName = read_input("Specify the name of a new directory\n");
    if(send_command('e') != 0)
        return 1;
    if(server.send(folderName, 100) != sf::Socket::Done)
        return 1;
    return 0;
}

int Client::ask_change_directory(){
    //ask server to change current directory

    char* folderName = read_input("Where to?\n");
    if(send_command('f') != 0)
        return 1;
    if(server.send(folderName, 100) != sf::Socket::Done)
        return 1;
    return 0;
}

int Client::ask_lock_file(){
    //ask server to lock a file

    char* selectedFile = read_input("Which file should be locked?\n");
    char* newMask = read_input("0 - return\n1 - block writing\n2 - block reading\n");
    newMask[1] = '\0';
    if(send_command('g') != 0)
        return 1;
    if(server.send(selectedFile, 100) != sf::Socket::Done)
        return 1;
    if(server.send(newMask, 5) != sf::Socket::Done)
        return 1;
    return 0;
}

int Client::ask_unlock_file(){
    //ask server to unlock a file

    char* selectedFile = read_input("Which file should be unlocked?\n");
    char* newMask = read_input("0 - return\n1 - unlock writing\n2 - unlock reading\n");
    newMask[1] = '\0';
    if(send_command('h') != 0)
        return 1;
    if(server.send(selectedFile, 100) != sf::Socket::Done)
        return 1;
    if(server.send(newMask, 100) != sf::Socket::Done)
        return 1;
    return 0;
}

void Client::error_handler(int errCode){
    //after completing every command server sends information, if any problem occurred
    //if not, the server send value 0
    //errors, which occurred on the client's computer are also handled here

    char serverErrorCode[5];
    size_t received;
    if(server.receive(serverErrorCode, 5, received) != sf::Socket::Done)
        errCode = 1;
    else
        if(errCode == 0) //if there is no errors on our computer, take servers response
            errCode = (int)serverErrorCode[0];

    switch(errCode){
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
    case 11:
        cout << "Invalid syntax " << server.getRemoteAddress() << " on port: " << server.getLocalPort() << endl;
        break;
    }
    return;
}

void Client::wait_for_instruction(){
    //main loop in client's application, continuously asks user for commands and calls adequate functions

    char selection;
    int errorCode;
    while(true){
        cout << "> ";
        cin >> selection;

        switch(selection){
        case 'a':
            errorCode = ask_ls();
            break;
        case 'b':
            errorCode = ask_send_file();
            break;
        case 'c':
            errorCode = ask_receive_file();
            break;
        case 'd':
            errorCode = ask_delete_file();
            break;
        case 'e':
            errorCode = ask_make_directory();
            break;
        case 'f':
            errorCode = ask_change_directory();
            break;
        case 'g':
            errorCode = ask_lock_file();
            break;
        case 'h':
            errorCode = ask_unlock_file();
            break;
        case 'x':
            send_command('x');
            server.disconnect();
            cout << "Bye!" << endl;
            exit(0);
        default:
            errorCode = 0;
            cout << "Available commands:\n";
            cout << "a - show current directory\n";
            cout << "b - download file from the server\n";
            cout << "c - send file to the server\n";
            cout << "d - delete file\n";
            cout << "e - create a directory\n";
            cout << "f - change directory\n";
            cout << "g - lock a file\n";
            cout << "h - unlock a file\n";
            cout << "x - exit" << endl;
            continue;
        }

        error_handler(errorCode);
    }
}
