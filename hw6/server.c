#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>

#define BUF_SIZE 256
#define MAX_CLIENTS 100
#define MAX_MSG 100

typedef struct{
    int sock_fd;
    size_t client_id; //from 1 to MAX_CLIENTS
    char name[BUF_SIZE];
}client_t;

typedef struct{
    size_t head;
    size_t tail;
    size_t size;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    size_t sender_id[BUF_SIZE];
    char* messages[BUF_SIZE];
}msg_queue;

typedef struct{
    size_t size;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    client_t* client_array[MAX_CLIENTS];
}client_list;

void init_structures();
void* init_client(void* fd);
void welcome(client_t* client);
void goodbye(client_t* client);
void clear_buffer(char* buf);
void add_client(client_t* new_client);
void remove_client(size_t client_id);
void* broadcast(void* unused);
void enqueue(char* msg, size_t id);
void dequeue();

client_list clients;
msg_queue queue;

int main(int argc, char* argv[]) {
    int sock_fd, opt, port;
    
    struct sockaddr_in server_addr;
    
    if(argc == 2){
        port = (int) strtol(argv[1], (char**)NULL,10);
    }
    else{
        port = 8080;
    }
    
    
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd < 0){
        perror("Socket");
        exit(EXIT_FAILURE);
    }
    
    /*Initialize sockaddr_in server_addr*/
    opt = 1;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    if(bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("Bind");
        exit(EXIT_FAILURE);
    }
    
    if((listen(sock_fd, 3)) < 0){
        perror("Listen");
        exit(EXIT_FAILURE);
    }
    
    init_structures();
    
    pthread_t broadcast_tid;
    pthread_create(&broadcast_tid, NULL, broadcast, NULL);
    
    int tmp_sock_fd;
    
    while(1){
        socklen_t size = sizeof(server_addr);
        if((tmp_sock_fd = accept(sock_fd, (struct sockaddr*)&server_addr, (socklen_t*) &size)) < 0){
            perror("Accept");
            continue;
        }
        
        pthread_t client_tid;
        pthread_create(&client_tid, NULL, init_client, (void*)&tmp_sock_fd);
        
    }
    return 0;
} 

void init_structures(){
    memset(&clients, 0, sizeof(client_list));
    memset(clients.client_array, 0, MAX_CLIENTS);

    memset(&queue, 0, sizeof(msg_queue));
    memset(queue.messages, 0, MAX_MSG);
    memset(queue.sender_id, 0, MAX_MSG);
    
    queue.head = 0;
    queue.tail = 0;
    queue.size = 0;
    clients.size = 0;
    
    pthread_mutex_init(&(clients.lock), NULL);
    pthread_mutex_init(&(queue.lock), NULL);
    pthread_cond_init(&(clients.cond), NULL);
    pthread_cond_init(&(queue.cond), NULL);
}


/*
 * Fills client_t struct with data
 * Adds new client struct to client list
 * Reads from client socket and calls enqueue
 */
void* init_client(void* fd){
    pthread_detach(pthread_self());
    /* Welcome message*/
    char buf[BUF_SIZE];

    int* sock_fd = fd;

    if(read(*sock_fd, buf, BUF_SIZE) < 0){
        perror("read"); //end thread
    }


    /*Allocates new client to the heap*/
    client_t* client = (client_t*) malloc(sizeof(client_t));
    client->sock_fd = *sock_fd;
    strncpy(client->name, buf, strlen(buf)-2);
    
    welcome(client);
    add_client(client);

    char msg[BUF_SIZE];
    memset(&buf, 0, sizeof(buf));
    
    while(read(client->sock_fd, buf, BUF_SIZE) > 0){
        if(strcmp(buf, "DONE\n") == 0){
            break;
        }
        memset(&msg, 0, sizeof(msg));
        
        strncpy(msg, client->name, strlen(client->name));
        strncat(msg, ": ", 2);
        strncat(msg, buf, strlen(buf));
        enqueue(msg, client->client_id);

        memset(&buf, 0, sizeof(buf));
        
    }
    write(client->sock_fd, "Goodbye\n", strlen("GoodBye\n"));
    close(client->sock_fd);
    
    goodbye(client);
    remove_client(client->client_id);
    
    pthread_exit(0);
}

void welcome(client_t* client){
    char msg[BUF_SIZE];
    strncpy(msg, client->name, strlen(client->name));
    strncat(msg, " has joined the server\n", 25);
    enqueue(msg, client->client_id);
    
    pthread_mutex_lock(&(clients.lock));
    char users_online[BUF_SIZE];
    char* header = "Users online: \n";
    strncpy(users_online, header, strlen(msg));
    
    for(size_t i = 1; i < MAX_CLIENTS; i++){
        if(clients.client_array[i] != NULL){
            strncat(users_online, clients.client_array[i]->name,strlen(clients.client_array[i]->name));
            strncat(users_online, "\n", 1);
        }
    }
    
    write(client->sock_fd, users_online, strlen(users_online));
    pthread_mutex_unlock(&(clients.lock));
    
}
void goodbye(client_t* client){
    char msg[BUF_SIZE];
    strncpy(msg, client->name, strlen(client->name));
    strncat(msg, " has left the server\n", 23);
    enqueue(msg, client->client_id);
}

void add_client(client_t* new_client){
    /*Adds client to the from of client list*/
    
    pthread_mutex_lock(&(clients.lock));
    if(clients.size + 1 == MAX_CLIENTS)//reject at max capacity
        pthread_cond_wait(&(clients.cond), &(clients.lock));

    for(size_t i = 1; i < MAX_CLIENTS; i++){
        if(clients.client_array[i] == NULL){
            new_client->client_id = i;
            clients.client_array[i] = new_client;
            clients.size++;
            pthread_mutex_unlock(&(clients.lock));
            return;
        }
    }
    pthread_mutex_unlock(&(clients.lock));
    perror("add_client: Could not find available room");
}

void remove_client(size_t client_id){
    pthread_mutex_lock(&(clients.lock));
    free(clients.client_array[client_id]);
    clients.client_array[client_id] = NULL;
    clients.size--;
    
    pthread_cond_signal(&(clients.cond));
    pthread_mutex_unlock(&(clients.lock));
}

/*
* Consumer thread acquires lock on message queue
* When not empty, Broadcasts message from the head 
* of queue to all clients. Calls dequeue
*/
void* broadcast(void* unused){ //Consumer Thread
    pthread_detach(pthread_self());
    while(1){
        
        pthread_mutex_lock(&(queue.lock));
        while(queue.size == 0)//Waits for message queue to hold a message
            pthread_cond_wait(&(queue.cond), &(queue.lock));
        
        pthread_mutex_lock(&(clients.lock));
        
        size_t head = queue.head;
        for(size_t i = 1; i < MAX_CLIENTS; i++){
            /*Broadcast to all clients*/
            if(clients.client_array[i] != NULL && queue.sender_id[head] != i){
                if(write(clients.client_array[i]->sock_fd, queue.messages[head], BUF_SIZE) < 0){
                    perror("Write MSG");
                }
            }
        }
        
        dequeue();
        pthread_mutex_unlock(&(queue.lock));
        pthread_mutex_unlock(&(clients.lock));
    }
}

//Removes a message from the head of the queue
void dequeue(){
    size_t head = queue.head;
    free(queue.messages[head]);
    memset(queue.messages[head], 0, BUF_SIZE);
    queue.sender_id[head] = 0;
    queue.messages[head] = NULL;
    queue.head = (head + 1) % MAX_MSG;
    queue.size--;
}
/*
* Producer thread acquires lock on message queue
* Adds a message to the back of the queue when
* not empty
*/
void enqueue(char* msg, size_t id){
    pthread_mutex_lock(&(queue.lock));
    if(queue.size == MAX_MSG)
        pthread_cond_wait(&(queue.cond), &(queue.lock));
    
    size_t tail = queue.tail;
    
    char* tmp = (char*) malloc(BUF_SIZE);
    strncpy(tmp, msg, strlen(msg));
    
    queue.sender_id[tail] = id;
    queue.messages[tail] = tmp;
    tail = (tail + 1) % MAX_MSG;
    queue.tail = tail;
    queue.size++;
    pthread_cond_signal(&(queue.cond));
    pthread_mutex_unlock(&(queue.lock));
    //Signals there is a message in the message queue

}
