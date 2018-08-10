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

int main(int argc, char* argv[]){
    int sock;
    char buf_out[BUF_SIZE];
    char buf_in[BUF_SIZE];
    char name[BUF_SIZE];
    int opt = 1;
    char* ip_addr = "127.0.0.1";
    int port = 8080;
    
    if(argc > 1){
        strncpy(name, argv[1], strlen(argv[1]));
        strncat(name, "\r\n", 2);
        if(argc > 2){
            port = (int) strtol(argv[2], (char**)NULL,10);
            if(argc > 3){
                ip_addr = argv[3];
            }
        }
    }
    else{
        strncpy(name, "Client\r\n", strlen("Client\r\n"));
    }
    
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
    
    write(sock, name, strlen(name));
    
    fd_set read_fds;
    int ready;
    
    while(1){
        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        
        if((ready = select(sock+1, &read_fds, NULL, NULL, NULL)) < 0){continue;}
        
        if(FD_ISSET(sock, &read_fds)){
            read(sock, buf_in, BUF_SIZE);
            printf("%s", buf_in);
            memset(buf_in, 0, sizeof(buf_in));
        }
        else if(FD_ISSET(STDIN_FILENO, &read_fds)){
            if (fgets(buf_out, BUF_SIZE, stdin) == NULL) {
                perror("fgets");
                exit(EXIT_FAILURE);
            }
            char tmp[BUF_SIZE];
            memset(tmp, 0, BUF_SIZE);
            strncat(tmp, buf_out, strlen(buf_out));

            if (write(sock, tmp, strlen(buf_out)) < 0) {
                perror("send"); 
                exit(EXIT_FAILURE);
            }
            if(strcmp(tmp, "DONE\n") == 0){
                exit(0);
            }
            memset(buf_out, 0, sizeof(buf_out));
        }
    }
    return 0;
} 

