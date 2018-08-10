#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>
#include <fcntl.h>

#define BUF_SIZE 256

void set_nonblock(int sockfd);

int main(int argc, char* argv[]){
    int sock, new_sock;
    char buf_out[BUF_SIZE];
    char buf_in[BUF_SIZE];
    char name[BUF_SIZE];
    int opt = 1;
    char* ip_addr = "127.0.0.1";
    int port = 8080;
    if(argc > 1){
        strncpy(name, argv[1], strlen(argv[1]));
        if(argc > 2){
            port = (int) strtol(argv[2], (char**)NULL,10);
            if(argc > 3){
                ip_addr = argv[3];
            }
        }
    }
    else{
        strncpy(name, "Client", strlen("Client"));
    }
    
    
    struct sockaddr_in addr;
    struct sockaddr_in server;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        perror("Socket");
        exit(EXIT_FAILURE);
    }
    
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt))){
        perror("SetSockOpt");
        exit(EXIT_FAILURE);
    }
    
    
    if(inet_pton(AF_INET, ip_addr, &server.sin_addr) <= 0){
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }
    
    puts("Connecting...");
    if(connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0){
        perror("Connect");
        exit(EXIT_FAILURE);
    }
    puts("Accepted");
    set_nonblock(sock);
    
    fd_set read_fds,write_fds;
    int ready;
    
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_SET(sock, &read_fds);
    FD_SET(STDIN_FILENO, &read_fds);
    
    while(1){
        if((ready = select(sock+1, &read_fds, &write_fds, NULL, NULL)) < 0){continue;}
        
        if(FD_ISSET(sock, &read_fds)){
            recv(sock, buf_in, BUF_SIZE, 0);
            printf("\n%s\n", buf_in);
            printf("%s: ", name);
        }
        else if(FD_ISSET(STDIN_FILENO, &read_fds)){
            if (fgets(buf_out, BUF_SIZE, stdin) == NULL) {
                perror("fgets");
                exit(EXIT_FAILURE);
            }
            char tmp[BUF_SIZE];
            strncpy(tmp, name, strlen(name));
            strncat(tmp, ": ", 2 + strlen(name));
            strncat(tmp, buf_out, BUF_SIZE);
            if (send(sock, tmp, BUF_SIZE, O_NONBLOCK) < 0) {
                perror("send"); 
                exit(EXIT_FAILURE);
            }
        }
    }
    return 0;
} 


void set_nonblock(int sockfd){
    //Adds NON_BLOCK to exisitng access modes
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

