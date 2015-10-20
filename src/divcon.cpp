#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <iomanip>

#define DELTA 30	
using namespace std;


int tag; /* Tag para as mensagens */
MPI_Status status; /* Status de retorno */

int array_size;
int processes;
int* vetor;
int size_vector;

int father,SonLeft,SonRight;	

//bubble sort created by teacher
void bs(int n, int * vetor)
{
	int c =0, d, troca, trocou =1;
	while (c < (n-1) & trocou )
	{
	trocou = 0;
	for (d = 0 ; d < n - c - 1; d++)
	if (vetor[d] > vetor[d+1])
	{
	troca = vetor[d];
	vetor[d] = vetor[d+1];
	vetor[d+1] = troca;
	trocou = 1;
	}
	c++;
	}	
}

//Vectors merge method(created by teacher)
int *interleaving(int vetor[], int tam)
{
	int *vetor_auxiliar;
	int i1, i2, i_aux;

	vetor_auxiliar = (int *)malloc(sizeof(int) * tam);

	i1 = 0;
	i2 = tam / 2;

	for (i_aux = 0; i_aux < tam; i_aux++) {
		if (((vetor[i1] <= vetor[i2]) && (i1 < (tam / 2)))
		    || (i2 == tam))
			vetor_auxiliar[i_aux] = vetor[i1++];
		else
			vetor_auxiliar[i_aux] = vetor[i2++];
	}

	return vetor_auxiliar;
}


//root of the tree	
void root(int id)
{
	size_vector = array_size;
	cout << "Root process started. " << array_size << " elements." << endl;  
	int i;	
	tag = 1; 
	vetor = new int[array_size];
	for (i=0 ; i<array_size; i++){              /* init array with worst case for sorting  (teacher)*/
		vetor[i] = array_size-i;
	}
	
	double start = MPI_Wtime();
	
	//cout << "Root process created array. " << endl;  	

 	SonLeft = (2*id)+1; //rank of the son of left
   	SonRight = (2*id)+2; //rank of the son of right	
	
	//MPI_Get_count(&status, MPI_INT, &size_vector);	 
	
	size_vector = array_size;
	
	MPI_Send ( &vetor[0], size_vector/2, MPI_INT, SonLeft, tag, MPI_COMM_WORLD);  
		MPI_Send ( &vetor[size_vector/2], size_vector/2,MPI_INT, SonRight, tag, MPI_COMM_WORLD);  
	
	//cout << "Root process send. "<< size_vector/2 << endl;		

	MPI_Recv ( &vetor[0],size_vector/2, MPI_INT, SonLeft, MPI_ANY_TAG, MPI_COMM_WORLD, &status);    
		MPI_Recv ( &vetor[size_vector/2], size_vector/2, MPI_INT, SonRight, MPI_ANY_TAG, MPI_COMM_WORLD, &status);  

	
	//cout << "Root process rec. " << endl;
	
	vetor = interleaving(vetor, size_vector);
	
	//Ensure vector is sorted.
	for(int i = 0; i < array_size - 1; i++) if (vetor[i] > vetor[i+1])
	{
		cout << "Array did not sort properly." << endl;
	}
	delete vetor;

	double elapsed = MPI_Wtime() - start;
	cout << setprecision(4) << elapsed << endl;
}
	
void son(int id)
{
	int i;
	
	if(id%2 == 0){
		father = (id-2)/2;		
	}
	else{
		father = (id-1)/2;		
	}	
	SonLeft = (2*id)+1;
   	SonRight = (2*id)+2;	
	
	//cout << " id " << id <<" father " << father<< " Left " << SonLeft << " Right " << SonRight << endl;

	int l = 0;
 	int y;
	for(y = id; y > 0; y = (y-1)/2){
		 l = l + 1;
	}
	size_vector = (array_size/pow(2, l));
	vetor = new int[size_vector];	
	
	if(id %2 == 0){
		MPI_Recv(&vetor[0], size_vector, MPI_INT, father, MPI_ANY_TAG, MPI_COMM_WORLD, &status);			
	}
	else{
		MPI_Recv(&vetor[0], size_vector + 1, MPI_INT, father, MPI_ANY_TAG, MPI_COMM_WORLD, &status);		
	}	

	if( (size_vector <= DELTA || id*2+2 >= processes)){ //if size of the vector less than five(can be any number) conquest
		bs(size_vector, vetor);
		//cout << "Son bs finished. " << endl;			
		MPI_Send (&vetor[0], size_vector, MPI_INT, father, tag, MPI_COMM_WORLD); 		
	}
	else{//sends to the children and awaits the result, than send result to father					
		MPI_Send ( &vetor[0], size_vector/2, MPI_INT, SonLeft, tag, MPI_COMM_WORLD);
		if(size_vector %2 == 0){
        		MPI_Send ( &vetor[size_vector/2], size_vector/2,MPI_INT, SonRight, tag, MPI_COMM_WORLD);  
		}
		else{
			MPI_Send ( &vetor[(size_vector/2)], (size_vector/2) + 1,MPI_INT, SonRight, tag, MPI_COMM_WORLD);
		}		
		//cout << "Son process send. " << endl;
		
		MPI_Recv ( &vetor[0],size_vector/2, MPI_INT, SonLeft, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		if(size_vector %2 == 0){    
        		MPI_Recv ( &vetor[size_vector/2], size_vector/2, MPI_INT, SonRight, MPI_ANY_TAG, MPI_COMM_WORLD, &status);						
		}
		else{
			MPI_Recv ( &vetor[(size_vector/2)], (size_vector/2) + 1, MPI_INT, SonRight, MPI_ANY_TAG, MPI_COMM_WORLD, &status);				
		}	
		
		//cout << "Son process rec. " << endl;

		vetor = interleaving(vetor, size_vector);		

		//cout << "id teste" << id << endl;
		//for (i=0 ; i<size_vector; i++)
		//{	        
		//printf("%d, ",vetor[i]);				
		//}
		//cout << endl;

		//cout << "Son process interleaving. " << endl;

		MPI_Send (&vetor[0], size_vector, MPI_INT, father, tag, MPI_COMM_WORLD);   
	}
	delete vetor;
}
	
int main(int argc, char* argv [])
{	
	MPI_Init(&argc, &argv);
	array_size = atoi(argv[1]);
	int rank;	

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &processes);

	if(rank == 0) root(rank);
	    else son(rank);

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
}
