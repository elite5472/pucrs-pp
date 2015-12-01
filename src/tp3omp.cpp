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
const int DONE_CALL = 100000000;

void bs(int n, int * vetor)
{
    int c =0, d, troca, trocou =1;
    while (c < (n-1) & trocou )
        {
        trocou = 0;
        for (d = 0 ; d < n - c - 1; d++)
            if (vetor[d] > vetor[d+1])
                {
                troca      = vetor[d];
                vetor[d]   = vetor[d+1];
                vetor[d+1] = troca;
                trocou = 1;
                }
        c++;
        }
}

void master(int id, int process_count, int array_size, int bag_size)
{
	cout << "Master: Process started.\n";
	int** input = new int*[bag_size];
	int** output = new int*[bag_size];
	int sent = 0;
	int processed =  0;
	MPI_Status status;

	//Create worst case bubble sort:
	for (int i = 0; i < bag_size; i++) 
	{
		input[i] = new int[array_size];
		for (int j = 0; j < array_size; j++)
		{
			input[i][j] = array_size - j;
		}
	}

	//Start the clock
	double start = MPI_Wtime();
	
	while(sent < bag_size)
	{
		for(int i = 0; i < process_count; i++) if (i != id && sent < bag_size)
		{
			cout << "Master: senting " << sent << " to " << i << endl;
			//Send the array with its identifier id as tag.
			MPI_Send(input[sent], array_size, MPI_INT, i, sent, MPI_COMM_WORLD);
			sent++;
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
		int result[array_size];
		//Get processed jobs from slaves.
		MPI_Recv(result, array_size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		if(status.MPI_TAG >= 0 && status.MPI_TAG < bag_size)
		{
			output[status.MPI_TAG] = new int[array_size];
			processed++;
			for(int i = 0; i < array_size; i++)
			{
				output[status.MPI_TAG][i] = result[i];
			}
		}
	}
	cout << "All jobs complete." << endl;

	//All done.
	double elapsed = MPI_Wtime() - start;
    cout << "Elapsed: " << setprecision(4) << elapsed << endl;

	cout << "Cleaning up..." << endl;
	for(int i = 0; i < bag_size; i++)
	{
		delete input[i];
		delete output[i];
	}
	delete input;
	delete output;
}

void slave(int rank, int workers, int array_size)
{
	vector<int*>arrays;
	vector<int>ids;
	vector<int*>out_arrays;
	vector<int>out_ids;
	int receiver = -1;
	bool distributed = false;

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
		if(this_thread == receiver) while(!done)
		{
			MPI_Status status;
			int* result = new int[array_size];
			MPI_Recv(result, array_size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			#pragma omp critical(grab_item)
			{
				if(status.MPI_TAG == DONE_CALL)
				{
					distributed = true;
					done = true;
				}
				else
				{
					arrays.push_back(result);
					ids.push_back(status.MPI_TAG);
				}
			}
		}	//Once the done call is sent, receiver becomes a worker.

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
				bs(array_size, work_item);
				#pragma omp critical(put_item)
				{
					out_arrays.push_back(work_item);
					out_ids.push_back(work_item_id);
				}
			}
		}
	}
	
	for(int i = 0; i < out_arrays.size(); i++)
	{
		cout << rank  << " sending " << out_ids.back() << endl;
		MPI_Send(out_arrays.back(), array_size, MPI_INT, 0, out_ids.back(), MPI_COMM_WORLD);
		out_arrays.pop_back();
		out_ids.pop_back();
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
    MPI_Barrier(MPI_COMM_WORLD);
	return 0;
}
