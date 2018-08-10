#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_ARGS 128
#define MAX_LEN 1024

void parse(char** argv, char* line);
void execute(char** argv);
void changeDirectory(char** argv);
void redirect(char** argv);

int main(){
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    
    char* argv[MAX_ARGS];
    char line[MAX_LEN];
    char currDir [MAX_LEN];
    
    char* prompt = getenv("PS1");
    if(prompt == NULL){
        prompt = "$";
    }
    
    while(1){
        getcwd(currDir, MAX_LEN);
        printf("%s %s ", currDir, prompt);
        
        if(fgets(line, MAX_LEN, stdin) == NULL){
            perror("fgets");
            exit(1);
        }

        if(strlen(line) > 1){// Checks for empty line
            line[strlen(line)-1] = '\0'; //fgets appends a new line character
            parse(argv, line);
            execute(argv);
        }
        
    }
    return 0;
}

void parse(char** argv, char* line){
    char* token = strtok(line, " "); //Tokenizes each argument in line
    argv[0] = token;
    
    for(int i = 1; token != NULL && i < MAX_ARGS; i++){
        token = strtok(NULL, " ");
        argv[i] = token;
    }
}

void execute(char** argv){
    int out = dup(1);
    redirect(argv);
    
    if(argv == NULL){
        exit(1);
    }
    
    if(!strcmp(argv[0], "exit")){
        close(out);
        exit(0);
    }
    else if(!strcmp(argv[0], "cd")){
        changeDirectory(argv);
    }
    else{
        pid_t pid = fork();
        
        if(pid < 0){
            printf("fork failed \n");
            exit(1);
        }
        else if(pid == 0){
            if(execvp(*argv, argv) < 0){
                printf("exec failed \n");
                exit(1);
            }
        }
        else{
            wait(NULL);
        }
    }
    dup2(out, 1);
    close(out);
}

void redirect(char** argv) {
	for (int i = 0; argv[i] != NULL; i++) {

		if (!strcmp(argv[i], ">")) { // Out

			int out = open(argv[i + 1], O_CREAT | O_WRONLY | O_TRUNC, 0666); 
			if (out < 0) {
				perror("file error");
			}
			if (dup2(out, 1) < 0) {
				perror("dup2 failed");
			}

			argv[i] = argv[i+1];
            argv[i] = NULL;
			close(out);
			break;
		}
		
        if (!strcmp(argv[i], "<")) { // In
			int in = open(argv[i + 1], O_RDONLY);
			if (in < 0) {
				perror("file error");
			}
			if (dup2(in, 0) < 0) {
				perror("dup2 failed");
			}

			argv[i] = argv[i+1];
            argv[i+1] = NULL;
			close(in);
			break;
		}
	
	}

}

void changeDirectory(char** argv){
    if(argv[1] == NULL){
        if(chdir(getenv("HOME")) == -1){
            perror("chdir");
        }
    }
    else{
        if(chdir(argv[1]) == -1){
            perror("chdir");
        }
    }
}
