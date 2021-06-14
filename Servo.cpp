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

Servo::Servo(int portNumber){
    //start new server and wait for new clients on the specified port

    listener.listen(portNumber);
    if(listener.accept(client) != sf::Socket::Done){
        cout << "There was an error while connecting new client" << endl;
        pthread_exit(0);
    }
    error_handler(wait_for_password());
    this->update_fs();
}

Servo::~Servo(){
    //delete files structure and exit thread for this client

    for(unsigned int i=0; i<filesList.size(); ++i)
        delete filesList[i];
    delete_maskfile();
    pthread_exit(0);
}

string Servo::make_windows_path(string s){
    //convert path from Unix to Windows format

    for(unsigned int i=0; i<s.size(); ++i)
        if(s[i] == '/')
            s[i] = '\\';
    return s;
}

string Servo::make_linux_path(string s){
    //convert path from Windows to Unix format

    for(unsigned int i=0; i<s.size(); ++i)
        if(s[i] == '\\')
            s[i] = '/';
    return s;
}

int Servo::chars_to_int(char* arr){
    //convert a number stored as an array of chars into int value
    //arr - array

    string s = arr;
    int ret = 0;
    for(unsigned int i=0; i<s.size(); ++i){
        ret = ret*10 + arr[i]-'0';
    }
    return ret;
}

char* Servo::int_to_chars(int val){
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

string Servo::hash_password(string pas){
    //returns a hashed value of given password

    return sha256(pas);
}

void Servo::lock_maskfile(){
    //locks file handling file accesses - only one thread can modify it, other thread need to wait

    while(!check_maskfile_lock()){
        string command = "echo . > " + maskfile + maskfileLock;
        system(command.c_str());
    }
}

void Servo::unlock_maskfile(){
    //when thread ends modifying masks, it removes lock on the maskfile
    string command;
#ifdef _WIN32
        command = "del ";
# else
        command = "rm ";
#endif // _WIN32
    command += maskfile + maskfileLock;
    system(command.c_str());
}

void Servo::delete_maskfile(){
    //deletes mask file

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
    //checks if maskfile is locked

    ifstream f(maskfile + maskfileLock);
    return f.good();
}

char Servo::check_mask(string filepath){
    //checks mask of a specified file, if file does not have any locks on it, function returns '0'

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
    //sets a new mask on a specified file and saves it to the maskfile
    //if there already was a lock on the file, its mask is updated

    lock_maskfile();
    char currMask = '0';
    fstream f;

#ifdef _WIN32
    f.open(maskfile, ios_base::in);
#else
    f.open(maskfile, ios::in);
#endif // _WIN32

    string s;
    while(f >> s){
        if (s.substr(0, s.size()-1) == filepath){
            currMask = s[s.size()-1];
        }
    }
    currMask -= '0';
    currMask = currMask | mask;
    currMask += '0';

    f.close();
    string temp = "echo " + filepath + currMask + " >> " + (string)maskfile;
    system(temp.c_str());
    unlock_maskfile();
}


void Servo::unset_mask(string filepath, char mask){
    //unset mask for specified file and saves new mask to the maskfile

    lock_maskfile();
    char currMask = '0';
    fstream f;

#ifdef _WIN32
    f.open(maskfile, ios_base::in);
#else
    f.open(maskfile, ios::in);
#endif // _WIN32

    string s;
    while(getline(f, s)){
        if (s.substr(0, s.size()-2) == filepath){
            currMask = s.back();
        }
    }

    currMask -= '0';
    currMask = currMask & ~mask;
    currMask += '0';

    f.close();
    string temp = "echo " + filepath + currMask + " >> " + (string)maskfile;
    system(temp.c_str());
    unlock_maskfile();
}

void Servo::update_fs(){
    //reads all files and updates filesList, function also updates the current size of all user's files

    lock_maskfile();
    for(unsigned int i=0; i<filesList.size(); ++i)
        delete filesList[i];
    int filesSize = 0;
    filesList.clear();

#ifdef _WIN32
    string systemPath = ExePath();
    system("dir /S /B > temp.txt");
    fstream f;
    f.open("temp.txt", ios_base::in);
    string s;
    while(f >> s){
        if(s.size() <= systemPath.size())
            continue;
        s = s.substr(systemPath.size(), MAX_PATH);
        s = make_linux_path(s);
        int pos = s.find_last_of("/")+1;
        MyFile* newFile = new MyFile;
        newFile->path = s.substr(0,pos);
        newFile->name = s.substr(pos, s.size());
        newFile->isDir = newFile->name.find(".") == string::npos ? true : false;
        filesList.push_back(newFile);
        int len = userName.size();
        if(newFile->path.substr(0,len) == userName){
            struct stat sb;
            stat(make_windows_path(s).c_str(), &sb);
            filesSize += sb.st_size;
        }
    }
    f.close();
    system("del temp.txt");
    currentSize = filesSize;

#else
    system("ls -1R > temp.txt");

    fstream f;
    f.open("temp.txt", ios::in);
    string relativePath, s;
    while(f >> s){
        if(s[0] == '.'){
            if(s.size() == 2){
                relativePath = "";
            } else {
                relativePath = s.substr(2, s.size()-3);
                relativePath += "/";
            }
            continue;
        }
        MyFile* newFile = new MyFile;
        newFile->name = s;
        newFile->path = relativePath;
        newFile->isDir = s.find(".") == string::npos ? true : false;
        filesList.push_back(newFile);
        int len = userName.size();
        if(newFile->path.substr(0,len) == userName){
            struct stat sb;
            s = path + s;
            if(stat(s.c_str(), &sb) == -1)
                continue;
            filesSize += sb.st_size;
        }
    }
    f.close();
    system("rm temp.txt");
    currentSize = filesSize;


#endif // _WIN32
    unlock_maskfile();
}

int Servo::wait_for_password(){
    //wait for client to send username and password, than check if it is correct
    //if yes start working with client, if not - try 3 times and the disconnect

    for(int i=0; i<3; ++i){
        char user[100];
        char pass[100];
        size_t received;
        if(client.receive(user, 100, received) != sf::Socket::Done){
            return 1;
        }
        if(client.receive(pass, 100, received) != sf::Socket::Done){
            return 1;
        }

        if(check_password(user, pass)){
            this->path = user;
            this->path += "/";
            this->userName = user;

            //log event
            time_t currentTime = chrono::system_clock::to_time_t(chrono::system_clock::now());
            string tempCommand = (string)ctime(&currentTime) + " user " + userName + " logged in successfully from ip " + client.getRemoteAddress().toString();
            cout << tempCommand << '\n';
            tempCommand = "echo " + tempCommand + " >> server.logs";
            system(tempCommand.c_str());

            return 0;
        }else{
            error_handler(2);
        }
    }
    return 2;
}

bool Servo::check_password(string user, string passwd){
    //check if username and password are correct

    fstream f;
    f.open("passwd.txt", ios::in);
    if(!f.is_open()){
        cout << "Error no passwd file" << endl;
        exit(1);
    }

    string readUser;
    string readHash;
    string readSalt;
    while(f >> readUser >> readHash >> readSalt){
        if (user == readUser){
            string hashValue = sha256(readSalt + passwd);
            if (hashValue == readHash){
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
    //send list of files in current directory to the client

    char ls[1000];
    int w=0;
    for(unsigned int i=0; i<filesList.size() && w<999; ++i){
        if(filesList[i]->path == path){
            for(unsigned int j=0; j<filesList[i]->name.size() && w<999; ++j){
                ls[w++] = filesList[i]->name[j];
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
    //returns true if file already exists, if not - returns false

    for(unsigned int i=0; i<filesList.size(); ++i)
        if(filesList[i]->path + filesList[i]->name == path + s){
            return true;
        }

    return false;
}

int Servo::send_file(){
    //send file to the client

    char np[100];
    size_t received;
    if(client.receive(np, 100, received) != sf::Socket::Done) //receive from client, a name of a file
        return 1;
    string fileName = np;

    //check if name of the file is correct
    if(fileName.find("/") != string::npos || fileName.find(".") == string::npos){
        if(client.send(int_to_chars(fileName.size()), 100) != sf::Socket::Done) //send to client the size of the file
            return 1;
        if(client.send(fileName.c_str(), fileName.size()) != sf::Socket::Done) //send the file to the client
            return 1;
        return 11;
    }

    fstream f;
    set_mask(path+fileName, 2); //sets lock on the file
    f.open(path+fileName, fstream::in);
    if(!f.is_open()){
        return -1;
    }
    //calculates the size of the file
    long beg, fin;
    beg = f.tellg();
    f.seekg(0, ios::end);
    fin = f.tellg();
    int fileSize = fin-beg;
    //moves to the beginning of the file
    f.clear();
    f.seekg(0, ios::beg);

    char arr[fileSize+1];
    int w=0;
    while(!f.eof()){
        f.get(arr[w++]);
    }
    f.close();
    unset_mask(path+fileName, 2);
    arr[fileSize] = '\0';

    if(client.send(int_to_chars(fileSize), 100) != sf::Socket::Done) //send to client the size of the file
        return 1;
    if(client.send(arr, fileSize) != sf::Socket::Done) //send the file to the client
        return 1;

    time_t currentTime = chrono::system_clock::to_time_t(chrono::system_clock::now());
    string tempCommand = (string)ctime(&currentTime) + " user " + userName + " downloaded file " + path+fileName;
    cout << tempCommand << '\n';
    tempCommand = "echo " + tempCommand + " >> server.logs";
    system(tempCommand.c_str());

    return 0;
}

int Servo::receive_file(){
    //receive file from client and save it

    char np[100];
    size_t received;
    if(client.receive(np, 100, received) != sf::Socket::Done) //file name
        return 1;
    string fileName = np;
    if(client.receive(np, 100, received) != sf::Socket::Done) //file size
        return 1;

    int fileSize = chars_to_int(np);
    char fileContent[fileSize+1];

    if(client.receive(fileContent, fileSize, received) != sf::Socket::Done)
        return 1;
    if(fileName.find("/") != string::npos || fileName.find(".") == string::npos) //check if name is correct
        return 11;
    if(exist_file(fileName)) //error if name already taken
        return 4;
    if(currentSize+fileSize > maxSize) //error if not enough space on the server
        return 9;

    fstream f;
    f.open(path+fileName, fstream::out);
    if(!f.is_open()){
        return -1;
    }
    set_mask(path+fileName, 1);
    fileContent[fileSize] = '\0';
    f << fileContent;
    f.close();
    unset_mask(path+fileName, 1);

    time_t currentTime = chrono::system_clock::to_time_t(chrono::system_clock::now());
    string tempCommand = (string)ctime(&currentTime) + " received file " + path+fileName + " from user " + userName;
    cout << tempCommand << '\n';
    tempCommand = "echo " + tempCommand + " >> server.logs";
    system(tempCommand.c_str());

    this->update_fs();
    return 0;
}

int Servo::delete_file(){
    //delete file on the server

    char np[100];
    size_t received;
    if(client.receive(np, 100, received) != sf::Socket::Done) //file name
        return 1;
    string fileName = np;
    if(!exist_file(fileName)){
        return 3;
    }
    if(fileName.find("/") != string::npos){ //check if name is correct
        return 11;
    }
    string absPath = path + fileName;
    char mask = check_mask(absPath);

    //error if file is locked
    if(mask == 1) return 6;
    if(mask == 2) return 5;

    time_t currentTime = chrono::system_clock::to_time_t(chrono::system_clock::now());
    string tempCommand = (string)ctime(&currentTime) + " user " + userName + " deleted file " + path+fileName;
    cout << tempCommand << '\n';
    tempCommand = "echo " + tempCommand + " >> server.logs";
    system(tempCommand.c_str());

    for(unsigned int i=0; i<filesList.size(); ++i){
        if(filesList[i]->path + filesList[i]->name == absPath){
#ifdef _WIN32
            if(filesList[i]->isDir){
                absPath = "rmdir /Q/S " + make_windows_path(absPath);
            } else {
                absPath = "del " + make_windows_path(absPath);
            }
            system(absPath.c_str());
#else
            if(filesList[i]->isDir){
                absPath = "rm -r " + absPath;
            } else {
                absPath = "rm " + absPath;
            }
            system(absPath.c_str());
#endif
            this->update_fs();
            return 0;
        }
    }
    return -1;
}

int Servo::make_directory(){
    //create a new directory

    char np[100];
    size_t received;
    if(client.receive(np, 100, received) != sf::Socket::Done) //new directory name
        return 1;
    string folderName = np;
    if(exist_file(folderName)){ //error if name is taken
        return 4;
    }
    if(folderName.find("/") != string::npos || folderName.find(".") != string::npos){ //check if name is correct
        return 11;
    }
    string absPath = "mkdir " + path + folderName;
#ifdef _WIN32
    system(make_windows_path(absPath).c_str());
#else
    system(absPath.c_str());
#endif // _WIN32
    this->update_fs();

    time_t currentTime = chrono::system_clock::to_time_t(chrono::system_clock::now());
    string tempCommand = (string)ctime(&currentTime) + " user " + userName + " created new directory " + path+folderName;
    cout << tempCommand << '\n';
    tempCommand = "echo " + tempCommand + " >> server.logs";
    system(tempCommand.c_str());

    return 0;
}

int Servo::change_directory(){
    //change current directory

    char np[100];
    size_t received;
    if(client.receive(np, 100, received) != sf::Socket::Done) //nazwa nowego folderu
        return 1;
    string folderName = np;
    if(folderName == ".."){ //go up
        int depth=0;
        for(unsigned int i=0; i<path.size(); ++i)
            if(path[i] == '/')
                ++depth;
        if(depth < 2) //cannot go up
            return 7;
        int w = path.size()-1;
        path[w--] = '\0';
        while(w>=0 && path[w] != '/')
            path[w--] = '\0';
        return 0;
    }
    if(folderName.find("/") != string::npos || folderName.find(".") != string::npos){ //check if name correct
        return 11;
    }
    if(!exist_file(folderName))
        return 3;
    string absPath = path + folderName;
    for(unsigned int i=0; i<filesList.size(); ++i){
        if(filesList[i]->path + filesList[i]->name == absPath){
            if(!filesList[i]->isDir){
                return 8;
            }
            path += folderName + "/";
            return 0;
        }
    }
    return -1;
}

int Servo::lock_file(){
    //lock a specified file

    char np[100];
    size_t received;
    if(client.receive(np, 100, received) != sf::Socket::Done) //file name
        return 1;
    string fileName = np;
    if(client.receive(np, 5, received) != sf::Socket::Done) //mask
        return 1;

    if(!exist_file(fileName)){ //error if file does not exist
        return 3;
    }
    if(fileName.find("/") != string::npos || fileName.find(".") == string::npos){ //error if incorrect name
        return 11;
    }

    char mask = np[0] - '0';
    if(mask == 0)
        return 0;
    string absPath = path + fileName;
    for(unsigned int i=0; i<filesList.size(); ++i){
        if(filesList[i]->path + filesList[i]->name == absPath){
            set_mask(absPath, mask);

            time_t currentTime = chrono::system_clock::to_time_t(chrono::system_clock::now());
            string tempCommand = (string)ctime(&currentTime) + " user " + userName + " locked file " + absPath;
            cout << tempCommand << '\n';
            tempCommand = "echo " + tempCommand + " >> server.logs";
            system(tempCommand.c_str());

            return 0;
        }
    }
    return -1;
}

int Servo::unlock_file(){
    //unlocks specified file

    char np[100];
    size_t received;
    if(client.receive(np, 100, received) != sf::Socket::Done) //file name
        return 1;
    string fileName = np;
    if(client.receive(np, 5, received) != sf::Socket::Done) //mask
        return 1;

    if(!exist_file(fileName)){
        return 3;
    }
    if(fileName.find("/") != string::npos || fileName.find(".") == string::npos){
        return 11;
    }

    char mask = np[0] - '0';
    if(mask == 0)
        return 0;
    string absPath = path + fileName;
    for(unsigned int i=0; i<filesList.size(); ++i){
        if(filesList[i]->path + filesList[i]->name == absPath){
            unset_mask(absPath, mask);

            time_t currentTime = chrono::system_clock::to_time_t(chrono::system_clock::now());
            string tempCommand = (string)ctime(&currentTime) + " user " + userName + " unlocked file " + absPath;
            cout << tempCommand << '\n';
            tempCommand = "echo " + tempCommand + " >> server.logs";
            system(tempCommand.c_str());

            return 0;
        }
    }
    return -1;
}

void Servo::error_handler(int errCode){
    //handles error codes and sends it to the client
    //if there was no error, send 0 to the client

    if(errCode == -1)
        errCode = 10;
    char code_for_client[5];
    code_for_client[0] = (char)errCode;
    if(client.send(code_for_client, 5) != sf::Socket::Done)
        errCode = 1;

    if(errCode == 0) //no errors
        return;

    string temporaryCommand = "";

    if(userName != ""){
        time_t currentTime = chrono::system_clock::to_time_t(chrono::system_clock::now());
        temporaryCommand = (string)ctime(&currentTime) + " user " + userName + ": ";
        cout << ctime(&currentTime) << " user " << userName << ": ";
    }

    switch(errCode){
    case 1:
        temporaryCommand += "Connection error with client " + client.getRemoteAddress().toString();
        cout << "Connection error with client " << client.getRemoteAddress() << " on port: " << client.getLocalPort() << endl;
        pthread_exit(0);
    case 2:
        temporaryCommand += "Wrong login data " + client.getRemoteAddress().toString();
        cout << "Wrong login data " << client.getRemoteAddress() << " on port: " << client.getLocalPort() << endl;
        break;
    case 3:
        temporaryCommand += "Tried to access not existing file " + client.getRemoteAddress().toString();
        cout << "Tried to access not existing file " << client.getRemoteAddress() << " on port: " << client.getLocalPort() << endl;
        break;
    case 4:
        temporaryCommand += "Tried to modify already existing file " + client.getRemoteAddress().toString();
        cout << "Tried to modify already existing file " << client.getRemoteAddress() << " on port: " << client.getLocalPort() << endl;
        break;
    case 5:
        temporaryCommand += "Tried to access read-blocked file " + client.getRemoteAddress().toString();
        cout << "Tried to access read-blocked file " << client.getRemoteAddress() << " on port: " << client.getLocalPort() << endl;
        break;
    case 6:
        temporaryCommand += "Tried to access write-blocked file " + client.getRemoteAddress().toString();
        cout << "Tried to access write-blocked file " << client.getRemoteAddress() << " on port: " << client.getLocalPort() << endl;
        break;
    case 7:
        temporaryCommand += "Tried to overcome restrictions " + client.getRemoteAddress().toString();
        cout << "Tried to overcome restrictions " << client.getRemoteAddress() << " on port: " << client.getLocalPort() << endl;
        break;
    case 8:
        temporaryCommand += "Tried to open file as a directory " + client.getRemoteAddress().toString();
        cout << "Tried to open file as a directory " << client.getRemoteAddress() << " on port: " << client.getLocalPort() << endl;
        break;
    case 9:
        temporaryCommand += "Tried to exceed memory limit " + client.getRemoteAddress().toString();
        cout << "Tried to exceed memory limit " << client.getRemoteAddress() << " on port: " << client.getLocalPort() << endl;
        break;
    case 10:
        temporaryCommand += "Undefined behavior " + client.getRemoteAddress().toString();
        cout << "Undefined behavior " << client.getRemoteAddress() << " on port: " << client.getLocalPort() << endl;
        break;
    case 11:
        temporaryCommand += "Invalid syntax " + client.getRemoteAddress().toString();
        cout << "Invalid syntax " << client.getRemoteAddress() << " on port: " << client.getLocalPort() << endl;
        break;
    }
    temporaryCommand = "echo " + temporaryCommand + " >> sever.logs";
    system(temporaryCommand.c_str());
    return;
}

void Servo::wait_for_command(){
    //wait for client to send command and call adequate function

    char comm[5];
    int errorCode = 0;
    size_t received;

    while(true){
        if(client.receive(comm, 5, received) != sf::Socket::Done)
            error_handler(1);

        switch(comm[0]){
        case 'a':
            errorCode = send_ls();
            break;
        case 'b':
            errorCode = send_file();
            break;
        case 'c':
            errorCode = receive_file();
            break;
        case 'd':
            errorCode = delete_file();
            break;
        case 'e':
            errorCode = make_directory();
            break;
        case 'f':
            errorCode = change_directory();
            break;
        case 'g':
            errorCode = lock_file();
            break;
        case 'h':
            errorCode = unlock_file();
            break;
        case 'x':
            client.disconnect();
            listener.close();
            return;
        default:
            errorCode = -1;
        }
        error_handler(errorCode);
    }
}
