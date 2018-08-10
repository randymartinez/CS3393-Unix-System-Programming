#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>  
#include <stdbool.h>

extern char** environ;

int main(){
    char* testy;
    size_t len = strlen(environ[0]);
     testy = strdup(environ[0]);
    printf("%zu %zu", len, strlen(testy));
    
    size_t size = 0;
    for(; environ[size] != NULL; size++){}
    char** env = (char**)malloc(size);
    
    char* tmp = strdup(environ[0]);
    env[0] = tmp;
    puts(env[0]);
    
    
}
