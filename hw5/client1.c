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
#define port 1234

void set_nonblock(int sockfd);

int main(){
    int sock, new_sock;
    char buf_out[BUF_SIZE];
    char buf_in[BUF_SIZE];
    int opt = 1;
    struct sockaddr_in addr;
    struct sockaddr_in server;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        perror("Socket");
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEport,&opt, sizeof(opt))){
        perror("SetSockOpt");
        exit(EXIT_FAILURE);
    }
    
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    
    if(inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) <= 0){
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }
    
    puts("Connecting...");
    if(connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0){
        perror("Connect");
        exit(EXIT_FAILURE);
    }
    puts("Accepted");
    set_nonblock(new_sock);
    
    fd_set read_fds,write_fds;
    int ready;
    
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);

    FD_SET(sock, &read_fds);
    //FD_SET(new_sock, &write_fds);
    FD_SET(STDIN_FILENO, &read_fds);
    
    while(1){
        if((ready = select(sock+1, &read_fds, &write_fds, NULL, NULL)) < 0){continue;}
        
        if(FD_ISSET(sock, &read_fds)){
            read(sock, buf_in, BUF_SIZE);
            printf("Client: %s\n", buf_in); 
        }
        else if(FD_ISSET(STDIN_FILENO, &read_fds)){
            if (fgets(buf_out, BUF_SIZE, stdin) == NULL) {
                perror("Error: Failed to read stdin");
                exit(1);
            }
            if (write(sock, buf_out, BUF_SIZE) < 0) {
                perror("Error: Failed to write to client"); 
            }
        }
        //else if(FD_ISSET(S, &write_fds)){
            
        //}
            /**else if(FD_ISSET(i, &write_fds)){
                //puts("Ready to write");
                send(new_sock, buf_out, BUF_SIZE, 0);
            }**/
    }
    /**
    puts("Accepted");

    read(new_sock, buf_in, BUF_SIZE);
    puts(buf_in);
    
    char* msg = "From server: Hi there";
    send(new_sock, msg, strlen(msg), 0);**/
    return 0;
} 


void set_nonblock(int sockfd){
    //Adds NON_BLOCK to exisitng access modes
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

