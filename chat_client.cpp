#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <boost/regex.h>
#include <iostream>
#include <ctime>
#include <vector>
#include <thread>
#include <signal.h>

std::string username;
int socketerino;

class args{
public:
    std::string username;
    std::string IP;
    int port = 21011;

    int parse(int argc, char *argv[]){
        if(argc != 5){
            return -1;
        }
        std::string tmp;
        for(int i = 1; i < argc; i++){
            tmp = argv[i];
            if (tmp == "-i"){
                i++;
                IP = argv[i];
            }
            else if (tmp == "-u"){
                i++;
                username = argv[i];
            }
            else{
                return -1;
            }
        }
        return 0;
    }
};

void senduj(int client_socket, args argumenty){
    std::string input("");
    int sent = 0;
    while(1){
        input = "";
        std::getline(std::cin,input);
        if(input == ""){
            input = "";
            continue;
        }
        input = argumenty.username + ": " + input + "\r\n";
        sent = send(client_socket, input.c_str(), strlen(input.c_str()), 0);

    }
}

void recievuj(int client_socket){
    char buffer[512];
    bzero(buffer, 512);
    std::string buferino("");
    int res = 0;
    while (1){
        for (;;){
            res = recv(client_socket, buffer, 1,0);
            buferino = buferino + buffer;
            if (res <= 0)
                break;
            if(buffer[0] == '\r'){

                res = recv(client_socket, buffer, 1,0);
                buferino = buferino + buffer;
                if(buffer[0] == '\n'){
                    break;
                }
            }

        }
        std::cout << buferino;
        buferino = "";
    }
}

void sighandler(int signal){
    std::string logoutMessage("");
    int sent = 0;
    logoutMessage = username + " logged out\r\n";
    sent = send(socketerino, logoutMessage.c_str(), strlen(logoutMessage.c_str()), 0);
    close(socketerino);
    exit(0);
}


int main(int argc, char *argv[]){
    args argumenty;
    if(argumenty.parse(argc, argv) != 0){
        fprintf(stderr, "HELP: -u USERNAME -i SERVER_IP\n");
        return -1;
    }

    const char *server_hostname = argumenty.IP.c_str();
    int client_socket;
    int port_number = 21011;
    struct hostent *server;
    struct sockaddr_in server_address;
    socklen_t serverlen;

    if ((server = gethostbyname(server_hostname)) == NULL) {
        fprintf(stderr,"ERROR: no such host as %s\n", server_hostname);
        exit(EXIT_FAILURE);
    }

    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_address.sin_addr.s_addr, server->h_length);
    server_address.sin_port = htons(port_number);


    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
    {
        exit(EXIT_FAILURE);
    }

    if (connect(client_socket, (const struct sockaddr *) &server_address, sizeof(server_address)) != 0)
    {
        exit(EXIT_FAILURE);
    }
    username = argumenty.username;
    socketerino = client_socket;
    signal(SIGINT, sighandler);
    int sent = 0;
    std::string welcomeMessage = argumenty.username + " logged in\r\n";
    sent = send(client_socket, welcomeMessage.c_str(), strlen(welcomeMessage.c_str()), 0);

    std::thread t1(senduj,client_socket, argumenty);
    std::thread t2(recievuj, client_socket);
    t1.join();
    t2.join();
    close(client_socket);
}
