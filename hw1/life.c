#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* build(char* filename,unsigned long* rows,unsigned long* columns);
void printGrid(char* grid, const unsigned long* rows, const unsigned long* columns);
void generation(char** grid, const unsigned long* rows, const unsigned long* columns, const unsigned long* generations);
int countNeighbors(char* grid, const unsigned long* rows, const unsigned long* columns, const size_t* index);

int main(int argc, char* argv[]){
    //life rows columns filename generations
    unsigned long rows = 10;
    unsigned long columns = 10;
    char* filename = "life.txt";
    unsigned long generations = 10;
    char *dummy;

    if (argc == 5){
        generations = strtol(argv[4],&dummy,10);
        filename = argv[3];
        columns = strtol(argv[2],&dummy,10);
        rows = strtol(argv[1],&dummy,10);
    }
    else if (argc == 4){
        filename = argv[3];
        columns = strtol(argv[2],&dummy,10);
        rows = strtol(argv[1],&dummy,10);
    }
    else if (argc == 3){
        columns = strtol(argv[2],&dummy,10);
        rows = strtol(argv[1],&dummy,10);
    }
    else if (argc == 2){
        rows = strtol(argv[1],&dummy,10);
    }
    
	char * grid = build(filename, &rows, &columns);
    generation(&grid, &rows, &columns, &generations);
    free(grid);
	return 0;
    
}
char* build(char* filename, unsigned long* rows, unsigned long* columns){
    //Generates the game world grid
    FILE* lifeFile;
	lifeFile = fopen(filename, "r");
	if(lifeFile == NULL){
		perror("Error while openning file.\n");
		exit(EXIT_FAILURE);
	}
    
    
    unsigned long fileRows = 0;
    unsigned long fileColumns = 0;
    int xctr = 0;
    int c;
    while((c = fgetc(lifeFile)) != EOF){ 
        //Counts the rows and columns of the file
        if (c == '\n'){
            fileRows++;
            if(fileColumns < xctr){
                fileColumns = xctr;
            }
            xctr = 0;
        }
        else{
            xctr++;
        }
    }
    fclose(lifeFile);
    
    if (fileRows > *rows){ 
        //If the file has more rows, replace rows
        *rows = fileRows;
    }
    if (fileColumns > *columns){
        //If the file has more columns, replace columns
        *columns = fileColumns;
    }
    
    char* grid = malloc((*rows) * (*columns) * sizeof(char)); 
    //Initialize a 1 dimensional array that represents the gameworld
    size_t size = (*rows) * (*columns);
    for(size_t pos = 0; pos < size+1; pos++){
        //Creates empty gameworld
        grid[pos] = '-';
    }
    
    lifeFile = fopen(filename, "r");
	if(lifeFile == NULL){
		perror("Error while openning file.\n");
		exit(EXIT_FAILURE);
	}
	
	fileRows = 0;
    fileColumns = 1;
    while((c = fgetc(lifeFile)) != EOF){ 
        //Places cells into grid
        if(c == '*'){
            grid[(fileRows)*(*columns) + fileColumns] = '*';
            fileColumns++;
        }
        else if (c == '\n'){
            fileRows++;
            fileColumns = 1;
        }
        else{
            fileColumns++;
        }
    }
    fclose(lifeFile);
    grid[size] = '\0';
    return grid;
}
void printGrid(char* grid, const unsigned long* rows, const unsigned long* columns){
    //Prints all cells in the game world
    for(size_t i = 1; i < (*rows)*(*columns)+1; i++){
        printf("%c", grid[i]);
        if(i%(*columns) == 0){
            printf("\n");
        }
    }
    printf("\n================================\n");
}
int countNeighbors(char* grid, const unsigned long* rows, const unsigned long* columns, const size_t* index){
    //Counts neighboring live cells
    int count = 0;
    if (((*index)-1)%(*columns) != 0 && (*index)-1 > 0 && grid[(*index)-1] == '*'){
        //Left of cell
        count++;
    }
    if (((*index)+1)%(*columns) != 1 && (*index)+1 < (*rows) * (*columns) && grid[(*index)+1] == '*'){//Right of cell
        //printf("RIGHT %zu\n", *index);
        count++;
    }
    if ((*index)-(*rows) > 0 && grid[(*index)-(*rows)] == '*'){
        //Above cell
        count++;
    }
    if (((*index)-(*rows)-1)%(*columns) != 0 && (*index)-(*rows)-1 > 0 && grid[(*index)-(*rows)-1] == '*'){
        //ABOVE left of cell
        count++;
    }
    if (((*index)-(*rows)+1)%(*columns) != 1 && (*index)-(*rows)+1 < (*rows) * (*columns)  && grid[(*index)-(*rows)+1] == '*'){
        //ABOVE right of cell
        count++;
    }
    if ((*index)+(*rows) < (*rows) * (*columns)  && grid[(*index)+(*rows)] == '*'){
        //BELOW cell
        count++;
    }
    if (((*index)+(*rows)-1)%(*columns) != 0 && (*index)+(*rows)-1 < (*rows) * (*columns) && grid[(*index)+(*rows)-1] == '*'){
        //BELOW left of cell
        count++;
    }
    if (((*index)+(*rows)+1)%(*columns) != 1 && (*index)+(*rows)+1 < (*rows) * (*columns)  && grid[(*index)+(*rows)+1] == '*'){
        //BELOW right of cell
        count++;
    }
    return count;
}
void generation(char** grid, const unsigned long* rows, const unsigned long* columns, const unsigned long* generations){
    printf("Generation 0:\n");
    printGrid(*grid, rows, columns);
    //Prints out the first generation
    char* nextGrid = malloc((*rows) * (*columns) * sizeof(char));
    nextGrid[(*rows)*(*columns)+1] = '\0';

    for(size_t numOfGeneration = 1; numOfGeneration < *generations+1; numOfGeneration++){
        for(size_t pos = 1; pos < (*columns)*(*rows)+1; pos++){
            //Iterate through all cells and generate new cells for the next generation
            int count = countNeighbors(*grid, rows, columns, &pos);
            if((*grid)[pos] == '*'){
                
                if(count < 2){
                    //Cells with less than 2 neighbors will die
                    nextGrid[pos] = '-';
                }
                else if(count < 4){
                    //Cells with 2 or 3 neighbors will move to the next generation
                    nextGrid[pos] = '*';
                }
                else{
                    //Cells with more than 3 neighbors will die
                    nextGrid[pos] = '-';
                }

            }
            else if(count == 3){
                //Dead cells with 3 live neighbors will live in the next generation
                nextGrid[pos] = '*';
            }
            else{
                //Default cells are dead cells
                nextGrid[pos] = '-';
            }
        }
        printf("Generation %zu:\n", numOfGeneration);
        printGrid(nextGrid, rows, columns);
        strncpy(*grid, nextGrid, ((*rows) * (*columns)+1));
        
    }
    
    free(nextGrid);
}



