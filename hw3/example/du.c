#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> 
#include <errno.h>

typedef struct{
    //Holds unique inode numbers
    size_t* array;
    size_t size;
    size_t capacity;
} inodes;

size_t traverse(char* dir_name, inodes* inode_arr);
size_t fileSize(struct stat* buf, inodes* inode_arr);

int main(int argc, char* argv[]){
    inodes inode_arr;
    inode_arr.capacity = 20;
    inode_arr.size = 0;
    inode_arr.array = malloc(sizeof(size_t) * inode_arr.capacity);
    
    if(argc == 1){
        //Traverse from root
        size_t size = traverse(".", &inode_arr);
        printf("%zu \t %s \n", size, "."); 
    }
    else if(argc == 2){
        //Traverse from given directory
        size_t size = traverse(argv[1], &inode_arr);
        printf("%zu \t %s \n", size, argv[1]); 
    }
    
    free(inode_arr.array);
    return 0;
}

size_t traverse(char* dir_name, inodes* inode_arr){
    //Traverses file system recursively, returns size of current directory
    DIR* directory;
    if((directory = opendir(dir_name)) == NULL){
        fprintf(stderr, "du: cannot read directory '%s':%s\n", dir_name,strerror(errno));
        return 0;
    }
    size_t total_size = 0;
    struct dirent* curr_dir;
    char* path;
    int path_len = 0;
    int name_len = 0;
    
    while((curr_dir = readdir(directory)) != NULL){
        if(strcmp(curr_dir->d_name, ".") != 0 && strcmp(curr_dir->d_name, "..") != 0){
            name_len = strlen(curr_dir->d_name);
            path_len = strlen(dir_name) + name_len + 2;
            path = (char*) calloc(path_len, sizeof(char));
            
            if(path == NULL){
                free(path);
                perror("Calloc failed \n");
                exit(1);
            }
            
            //Build full directory path
            strncat(path, dir_name, strlen(dir_name));
            strncat(path, "/", 1);
            strncat(path, curr_dir->d_name, name_len);
            
            struct stat buf;
            if(lstat(path,&buf)){
                perror("lstat failed\n");
                exit(1);
            }

            if(S_ISDIR(buf.st_mode)){
                //Recusively call traverse if curr_dir is another directory
                size_t size = traverse(path, inode_arr);
                printf("%zu \t %s\n", size, path);
                total_size += size;
            }
            else{
                total_size += fileSize(&buf, inode_arr);
            }
        }
    }
    free(path);
    //puts(path);

    closedir(directory);
    total_size += 4; //Size of own directory
    return total_size;
}

size_t fileSize(struct stat* buf, inodes* inode_arr){
    //Return the size of unique inodes
    size_t inode_num = buf->st_ino;
    
    if(buf->st_nlink > 1){
        for(int pos = 0; pos < inode_arr->size; pos++){
            if(inode_arr->array[pos] == inode_num){
                return 0;
            }
        }
    }
    
    inode_arr->array[inode_arr->size] = buf->st_ino;
    inode_arr->size++;
    
    if(inode_arr->capacity == inode_arr->size){
        //Resize inode array
        inode_arr->capacity *= 2;
        inode_arr->array = (size_t*) realloc(inode_arr->array, sizeof(size_t) *inode_arr->capacity);
        if(inode_arr->array == NULL){
            perror("Malloc failed\n");
            exit(1);
        }
    }
    return buf->st_blocks/2; //st_blocks consistently returned twice as much as du
}
