#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <iomanip>
#include <omp.h>
#include <vector>

using namespace std;

const bool debug = true;
const int chunk_size = 1000;
const int DONE_CALL = 100000000;

int **alloc_2d_int(int rows, int cols) {
    int *data = (int *)malloc(rows*cols*sizeof(int));
    int **array= (int **)malloc(rows*sizeof(int*));
    for (int i=0; i<rows; i++)
        array[i] = &(data[cols*i]);

    return array;
}

int compare_numbers(const void* x, const void* y){
	return (*(int*)x - *(int*)y);
}

void master(int id, int process_count, int array_size, int bag_size)
{
	cout << "Master: Process started.\n";
	int** input = alloc_2d_int(bag_size, array_size);
	int** output = alloc_2d_int(bag_size, array_size);
	int sent = 0;
	int processed =  0;
	MPI_Status status;

	//Create worst case bubble sort:
	for (int i = 0; i < bag_size; i++) for (int j = 0; j < array_size; j++)
	{
		input[i][j] = array_size - j;
	}

	//Start the clock
	double start = MPI_Wtime();
	
	while(sent < bag_size)
	{
		for(int i = 0; i < process_count; i++) if (i != id && sent < bag_size)
		{
			MPI_Send(&(input[0][0]) + sent, array_size * chunk_size, MPI_INT, i, sent, MPI_COMM_WORLD);
			sent = sent + chunk_size;
		}
	}
	for(int i = 0; i < process_count; i++) if (i != id)
	{
		int body = 0;
		//Send a done message to the process to stop listening for more jobs.
        MPI_Send(&body, 1, MPI_INT, i, DONE_CALL, MPI_COMM_WORLD);
	}
	cout << "All jobs sent." << endl;

	while(processed < bag_size)
	{
		int** result = alloc_2d_int(chunk_size, array_size);
		//Get processed jobs from slaves.
		MPI_Recv(&(result[0][0]), array_size*chunk_size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		if(status.MPI_TAG >= 0 && status.MPI_TAG < bag_size)
		{
			processed =  processed + chunk_size;
			for(int i = 0; i < chunk_size; i++) for(int j = 0; j < array_size; j++)
			{
				output[i + status.MPI_TAG][j] = result[i][j];
			}
		}
	}
	cout << "All jobs complete." << endl;

	//All done.
	double elapsed = MPI_Wtime() - start;
    	cout << "Elapsed: " << setprecision(4) << elapsed << endl;
}

void slave(int rank, int workers, int array_size)
{
	vector<int*>arrays;
	vector<int>ids;
	vector<int>chunks;
	vector<int*>out_arrays;
	vector<int>out_ids;
	int receiver = -1;
	bool distributed = false;
	double start = 0;
	
	#pragma omp parallel num_threads(workers)
	{
		int this_thread = omp_get_thread_num();
		int num_threads = omp_get_num_threads();
		bool done;
		
		//Choose which thread will be the receiver.
		#pragma omp critical(chose_receiver)
		{
			if(receiver == -1)
			{
				receiver = this_thread;
			}
		}


		//Receiver thread will fetch all jobs while other threads work on them.		
		done = false;
		bool timed = false;
		if(this_thread == receiver) while(!done)
		{
			MPI_Status status;
			int** result = alloc_2d_int(chunk_size, array_size);
			MPI_Recv(&(result[0][0]), array_size*chunk_size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			if(!timed)
			{
				timed = true;
				start = MPI_Wtime();
			}
			#pragma omp critical(grab_item)
			{
				if(status.MPI_TAG == DONE_CALL)
				{
					distributed = true;
					done = true;
				}
				else
				{
					//Save al the arrays in a queue for workers to grab from one by one, keeping
					//index information to rebuild the sorted bidimensional array as the output.]
					chunks.push_back(status.MPI_TAG);
					for(int i = 0; i < chunk_size; i++)
					{
						arrays.push_back(result[i]);
						ids.push_back(status.MPI_TAG + i);
					}
				}
			}
		}	//Once all the cunks of data are processed, receiver becomes a worker.

		done = false;
		int* work_item;
		int work_item_id = -1;
		while(!done)
		{
			#pragma omp critical(grab_item)
			{
				if(arrays.size() > 0)
				{
					work_item = arrays.back();
					arrays.pop_back();

					work_item_id = ids.back();
					ids.pop_back();
				}
				else
				{
					work_item_id = -1;
				}

				if(distributed && arrays.size() == 0) done = true;
			}
			if(work_item_id != -1)
			{
				qsort(work_item, array_size, 4, compare_numbers);
				#pragma omp critical(put_item)
				{
					out_arrays.push_back(work_item);
					out_ids.push_back(work_item_id);
				}
			}
		}
	}
	
	double elapsed = MPI_Wtime() - start;
	cout << "Slave Elapsed: " << setprecision(4) << elapsed << endl;

	//Prepare the output bidimensional array. Each chunk is an MPI sent call.
	for(int i = 0; i < chunks.size(); i++)
	{
		int** output = alloc_2d_int(chunk_size, array_size);
		//Search for the sorted arrays that are part of this chunk.
		for(int j = 0; j < out_arrays.size(); j++)
		{
			int rel_position = out_ids.at(j) - chunks.at(i);
			if (rel_position >= 0 && rel_position < chunk_size)
			{
				//Copy the array into the prepared memory structure.
				for(int k = 0; k < array_size; k++)
				{
					output[rel_position][k] = out_arrays.at(j)[k];
				}
			}
		}
		//Sent the chunk of sorted arrays to master.
		MPI_Send(&(output[0][0]), array_size*chunk_size, MPI_INT, 0, chunks.at(i), MPI_COMM_WORLD);
	}
}

int main(int argc, char* argv [])
{
	MPI_Init(&argc, &argv);
    int rank;
	int processes;
	int workers = atoi(argv[1]);
    int bag_size = atoi(argv[2]);
    int array_size = atoi(argv[3]);    

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &processes);
    
    if(rank == 0) master(0, processes, array_size, bag_size);
    else slave(rank, workers, array_size);
    MPI_Finalize();
	return 0;
}
