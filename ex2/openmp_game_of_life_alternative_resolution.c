#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <locale.h>

//square matrix side size
#define TABLE_SIZE 2048

//generations
#define GENERATIONS 5

//print generations by an interval of #GEN_PRINT_INTERVAL
//(ex: 5 -> (0,1,5,10,15,20,...,#GENERATIONS-1))
//(ex: 15 -> (0,1,15,30,45,60,...,#GENERATIONS-1))
#define GEN_PRINT_INTERVAL 1

//number of threads
#define NUM_THREADS 8

//value to use in srand()
#define SRAND_VALUE 1985

int alive_cells = 0; //total alive cells in one generation

struct timeval threads_start, threads_end;  //thread counters

//create matrixes
void gen_matrix(int ***M, int m, int n){
    *M = (int**) malloc(m * sizeof(int*));
    for(int i=0; i<n; i++){
        (*M)[i] = (int*) malloc(n * sizeof(int));
    }
}

//initialize matrixes with rand
void init_matrix(int **grid, int m, int n){
    srand(SRAND_VALUE);
    for(int i=0; i<m; i++){
        for(int j=0; j<n; j++){
            grid[i][j] = rand() % 2;
        }
    }
}

//copy matrix grid to matrix newgrid
void copy_matrix(int **grid, int **newgrid, int m, int n){
    for(int i=0; i<m; i++){
        for(int j=0; j<n; j++){
            grid[i][j] = newgrid[i][j];
        }
    }
}

void print_matrix(int **M, int m, int n){
    for(int i=0; i<m; i++){
        for(int j=0; j<n; j++){
            printf("%d ",M[i][j]);
        }
        printf("\n");
    }
}

//returns amount of neighbors alive
int getNeighbors(int** grid, int i, int j){

    int prev_row=0, next_row=0, prev_col=0, next_col=0, neighbor_alive=0;

    prev_row=(i-1+TABLE_SIZE)%TABLE_SIZE;
    next_row=(i+1)%TABLE_SIZE;
    prev_col=(j-1+TABLE_SIZE)%TABLE_SIZE;
    next_col=(j+1)%TABLE_SIZE;

    //neighbors_check
    if(grid[prev_row][prev_col] == 1) neighbor_alive++;
    if(grid[prev_row][j] == 1) neighbor_alive++;
    if(grid[prev_row][next_col] == 1) neighbor_alive++;
    if(grid[(i)][prev_col] == 1) neighbor_alive++;
    if(grid[(i)][next_col] == 1) neighbor_alive++;
    if(grid[next_row][prev_col] == 1) neighbor_alive++;
    if(grid[next_row][j] == 1) neighbor_alive++;
    if(grid[next_row][next_col] == 1) neighbor_alive++;

    //debug checker

                /*printf("I'm [%d][%d]", i, j);
                printf("\n [%d][%d] -> %d", prev_row, prev_col, grid[prev_row][prev_col]);
                printf("\n [%d][%d] -> %d", prev_row, j, grid[prev_row][j] == 1);
                printf("\n [%d][%d] -> %d", prev_row, next_col, grid[prev_row][next_col]);
                printf("\n [%d][%d] -> %d", i, prev_col, grid[(i)][prev_col]);
                printf("\n [%d][%d] -> %d", i, next_col, grid[(i)][next_col]);
                printf("\n [%d][%d] -> %d", next_row, prev_col, grid[next_row][prev_col]);
                printf("\n [%d][%d] -> %d", next_row, j, grid[next_row][j]);
                printf("\n [%d][%d] -> %d", next_row, next_col, grid[next_row][next_col]);
                printf("\nAlive: %d\n",neighbor_alive);
    */
    return neighbor_alive;
}

void game_of_life(int **p_grid, int **p_newgrid){

    int neighbor_alive=0, local_alive_cells=0, local_alive_cells_aux=0;

    int **grid =  p_grid;
    int **newgrid = p_newgrid;

    int i=0, j=0;

    omp_set_num_threads(NUM_THREADS);

    int thread_counter=0;

    gettimeofday(&threads_start, NULL);  //threads/main loop starts

    for (int gen=0; gen <= GENERATIONS; gen++){

        #pragma omp parallel for shared(thread_counter) private(j, neighbor_alive, local_alive_cells)
        for (i = 0; i < TABLE_SIZE; i++){

            local_alive_cells=0;

            for (j = 0; j < TABLE_SIZE; j++){

                if(grid[i][j]==1){
                    local_alive_cells=local_alive_cells+1;
                    //printf("\n[1] [%d][%d]##%d##\n",i,j,local_alive_cells);
                }

                neighbor_alive=getNeighbors(grid, i, j);

                //predefined rules
                //"Células vivas com menos de 2 (dois) vizinhas vivas morrem por abandono;
                // Cada célula viva com 2 (dois) ou 3 (três) vizinhos deve permanecer viva para a próxima geração;
                // Cada célula viva com 4 (quatro) ou mais vizinhos morre por superpopulação.
                // Cada célula morta com exatamente 3 (três) vizinhos deve se tornar viva."
                if(grid[i][j]==1){
                    if(neighbor_alive<2 || neighbor_alive>3) newgrid[i][j] = 0;
                    else newgrid[i][j] = grid[i][j];
                }
                else{
                    if(neighbor_alive == 3) newgrid[i][j] = 1;
                    else newgrid[i][j] = grid[i][j];
                }
            }
            //printf("\n[%d]: >Thread -> %d<\n", i, omp_get_thread_num());
            //printf("\n[3] ##%d##\n",local_alive_cells);

            //allow only 1 thread (fake semaphore)
            //on Windows 10 GCC 8.1, threads starts at 0 and goes until amount of threads declared
            while(thread_counter!=omp_get_thread_num());
            alive_cells += local_alive_cells; //one by one, threads add value to TOTAL alive_cells
            thread_counter++;

            if(thread_counter==NUM_THREADS) thread_counter=0;

        }

        if (gen == 0 && GENERATIONS==0) printf("Geração inicial (geração única)\t: %d células vivas\n", alive_cells);
        else if (gen == 0) printf("Geração inicial\t: %d células vivas\n", alive_cells);
        else if (gen == 1 && GENERATIONS>=2) printf("Geração %d\t: %d células vivas\n", gen, alive_cells);
        else if (gen == GENERATIONS) printf("Última geração\t: %d células vivas \t(%d iterações)", alive_cells, GENERATIONS);
        else if ((gen%GEN_PRINT_INTERVAL == 0) && (gen>=GEN_PRINT_INTERVAL)) printf("Geração %d\t: %d células vivas\n", gen, alive_cells);
        copy_matrix(grid, newgrid, TABLE_SIZE, TABLE_SIZE);

        if(alive_cells!=0){
            alive_cells=0;
        }

    }
    gettimeofday(&threads_end, NULL);  //threads/main loop ends
}

int main(){

    printf("System Specs");
    printf("\nProcessor: Intel Core i3-8100 (6 MB Cache / 4 Cores / 4 Threads)");
    printf("\nRAM: 16 GB / 2400 MHz\n");
    printf("OS: Windows 10\n\n");

    struct timeval start, end;  //counters

    gettimeofday(&start, NULL);    //program starts

    setlocale(LC_ALL, "");    //keyboard settings

    int **grid, **newgrid, m=0, n=0, elapsed_time=0, elapsed_time_threads=0;

    m=n=TABLE_SIZE; //rows (m) and columns (n)

    gen_matrix(&grid,m,n);
    gen_matrix(&newgrid,m,n);
    init_matrix(grid,m,n);

    game_of_life(grid, newgrid);

    elapsed_time_threads = ((threads_end.tv_sec * 1000000 + threads_end.tv_usec)
                            - (threads_start.tv_sec * 1000000 + threads_start.tv_usec))/1000;

    printf("\n\nThreads execution time: %u ms (THREADS: %d)", elapsed_time_threads, NUM_THREADS);

    gettimeofday(&end, NULL);
    elapsed_time = ((end.tv_sec * 1000000 + end.tv_usec)
                    - (start.tv_sec * 1000000 + start.tv_usec))/1000;

    printf("\nTotal execution time: %u ms\n", elapsed_time);

    return 0;
}
