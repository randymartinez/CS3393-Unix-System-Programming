#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <string.h>
#include <fcntl.h>

#define BUF_SIZE 256

void set_nonblock(int sockfd);

int main(int argc, char* argv[]) {
    int sock, new_sock;
    int opt = 1;
    char buf_in[BUF_SIZE];
    char buf_out[BUF_SIZE];
    char name[BUF_SIZE];
    int port = 8080;
    if(argc > 1){
        strncpy(name, argv[1], strlen(argv[1]));
        if(argc > 2){
            port = (int) strtol(argv[2], (char**)NULL,10);
        }
    }
    else{
        strncpy(name, "Server", strlen("Server"));
    }
    
    
    struct sockaddr_in addr;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        perror("Socket");
        exit(EXIT_FAILURE);
    }
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    if(bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        perror("Bind");
        exit(EXIT_FAILURE);
    }
    
    puts("Listening...");
    if((listen(sock, 3)) < 0){
        perror("Listen");
        exit(EXIT_FAILURE);
    }
    
    socklen_t size = sizeof(addr);
    if((new_sock = accept(sock, (struct sockaddr*)&addr, &size)) < 0){
        perror("Accept");
        exit(EXIT_FAILURE);
    }
    puts("Accepted");
   
    set_nonblock(new_sock);
    
    fd_set read_fds,write_fds;
    int ready;
    
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_SET(new_sock, &read_fds);
    FD_SET(STDIN_FILENO, &read_fds);
    
    while(1){
        
        if((ready = select(new_sock+1, &read_fds, &write_fds, NULL, NULL)) < 0){continue;}

        if(FD_ISSET(new_sock, &read_fds)){
            recv(sock, buf_in, BUF_SIZE,0);
            printf("\n%s\n", buf_in);
            printf("%s: ", name);
        }
        else if(FD_ISSET(STDIN_FILENO, &read_fds)){
            printf("%s: ", name);
            if (fgets(buf_out, BUF_SIZE, stdin) == NULL) {
                perror("fgets");
                exit(EXIT_FAILURE);
            }
            char tmp[BUF_SIZE];
            strncpy(tmp, name, strlen(name));
            strncat(tmp, ": ", 2 + strlen(name));
            strncat(tmp, buf_out, BUF_SIZE);
            if (send(new_sock, tmp, BUF_SIZE, O_NONBLOCK) < 0) {
                perror("Send");
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
