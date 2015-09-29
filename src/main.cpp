#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

using namespace std;

const int REQUEST_CALL = 0x63;
const int RETURN_CALL = 0x64;
const int TASK_CALL = 0x65;

int** arrays;
int array_count;
int array_size;
int processes;

int compare_numbers(const void* x, const void* y){
	return (*(int*)x - *(int*)y);
}

void master()
{
    double start = MPI_Wtime();
    int i = array_count;
    int done = 0;
    int* result = new int[array_size];
    MPI_Status status;
    
    //Keep track of who's working on what.
    int* process_index = new int[processes];
    for(int k = 0; k < processes; k++) process_index[k] = -1;
    
    //While no processes are halted.
    while(done < processes)
    {
        MPI_Recv(result, array_size, MPI_INT, MPI_ANY_SOURCE, REQUEST_CALL, MPI_COMM_WORLD, &status);
        
        //Replace the array if we know where to put it (and we should)
        if(status.MPI_SOURCE > 0 && process_index[status.MPI_SOURCE] != -1) memcpy(arrays[process_index[status.MPI_SOURCE]], result, 4*array_size);
        
        if(i == 0)
        {
            //No more work, halt.
            int body = 0
            MPI_Send(&body, 1, MPI_INT, status.MPI_SOURCE, RETURN_CALL, MPI_COMM_WORLD);
            done++;
        }
        else
        {
            //Send more work.
            i--;
            MPI_Send(&arrays[i], array_size, MPI_INT, status.MPI_SOURCE, TASK_CALL, MPI_COMM_WORLD);
            process_index[status.MPI_SOURCE] = i
        }
    }
    double elapsed = MPI_Wtime() - start;
    cout << "Elapsed: " << elapsed
}

void slave(int id)
{
    int* v = new int[array_size];
    MPI_Status status;
    
    while(true)
    {
        MPI_Send(v, array_size, MPI_INT, 0, REQUEST_CALL, MPI_COMM_WORLD);
        MPI_Recv(v, array_size, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, status);
        if(status.MPI_TAG == TASK_CALL)
        {
            qsort(v, array_size, 4, compare_numbers);
            cout << id << ": Sorted an array!";
        }
        else
        {
            cout << id << ": No more work. Halting.";
            return;   
        }
    }
}

int main(int argc, char* argv [])
{
    MPI_Init(&argc, &argv);
    
    int rank;
    array_count = atoi(argv[1]);
    array_size = atoi(argv[2 ]);
    
    arrays = new int*[array_count];
    for(int i = 0; i < array_count; i++)
    {
        arrays[i] = new int[array_size];
        for(int j = 0; j < array_size; j++)
        {
            arrays[i][j] = rand();
        }
    }
    
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &processes);
    
    processes--; //Discount the master process.
    
    if(rank == 0) master();
    else slave(rank);
}
