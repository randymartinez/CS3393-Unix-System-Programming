#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>  
#include <stdbool.h>
#include <malloc.h>

extern char** environ;

void insert(char* key, char* pair, char** newEnviron);

int main(int argc, char* argv[]){
    char** newEnviron;
    size_t size = 0;

    if(argc > 1){
        for(; environ[size] != NULL; size++){}//Counts the size of environ
        printf("%zu\n", size);
        newEnviron = malloc(60 * sizeof(char*));    
        size_t a =  malloc_usable_size(newEnviron);
        printf("%zu\n",a);
        int start = 1;
        if(strcmp(argv[1],"-i") == 0){    
            //If i flag is found, clear environ
            start++;
            printf("before nulls\n");
            for(size_t line = 0; environ[line] != NULL; line++){
                newEnviron[line] = NULL;
            }
            printf("After nulls\n");
        }
        else{
            printf("before Assignment\n");
            for(size_t line = 0; environ[line] != NULL; line++){
                printf("%zu \n", line);
                newEnviron[size] = strdup(environ[size]);
            }
            printf("After Assignment\n");
            newEnviron[size] = NULL;
        }
        printf("Before keyValue\n");
        char* key;
        char* value;
        char* copy;
        printf("Before Token Loop\n");
        for(size_t idx = start; idx < argc; idx++){
            //Tokenize each argument into key value pairs
            printf("Before Tokens\n");
            copy = strdup(argv[idx]);
            key = strtok(argv[idx], "=");
            value = strtok(NULL, "=");
            printf("After Tokens\n");
            if(value == NULL){
                //If there is no value, assume it is a command
                execve(key, argv+idx,newEnviron);
                //If the process does not terminate after execve, the command failed
                fprintf(stderr, "env: '%s' : No such file or directory\n", key);
                return 0;
            }
            printf("Before Insert\n");
            insert(key,copy, newEnviron);
        }
        free(copy);
    }
    for(size_t idx = 0; environ[idx] != NULL; idx++){
        puts(environ[idx]);
    }
    return 0;
} 

void insert(char* key, char* pair, char** newEnviron){
    //Inserts key value pairs into environ
    int keyLen = strlen(key);
    size_t line = 0;
    bool isFound = false;
    for(; newEnviron[line] != NULL && !isFound; line++){
        if(strlen(newEnviron[line]) < keyLen){
            continue;
        }
        for(size_t letter = 0; letter < keyLen; letter++){
            //Compares key from argv with every key in environ
            isFound = true;
            if(key[letter] != newEnviron[line][letter]){
                isFound = false;
                break;
            }
        }
    }
    if(isFound){
        //If found, replace key value pair
        newEnviron[line-1] = strdup(pair);
    }
    else{
        //If not found, append key value pair to the end of environ
        newEnviron[line] = strdup(pair);
        newEnviron[line+1] = NULL;
    }
}
