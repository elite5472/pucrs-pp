#!/bin/bash

ladrun -np 2 bin/tp3omp 1 10000 10000
ladrun -np 2 bin/tp3omp 2 10000 10000
ladrun -np 2 bin/tp3omp 4 10000 10000
ladrun -np 2 bin/tp3omp 8 10000 10000
ladrun -np 2 bin/tp3omp 16 10000 10000
ladrun -np 2 bin/tp3omp 32 10000 10000

ladrun -np 4 bin/tp3omp 1 10000 10000
ladrun -np 4 bin/tp3omp 2 10000 10000
ladrun -np 4 bin/tp3omp 4 10000 10000
ladrun -np 4 bin/tp3omp 8 10000 10000
ladrun -np 4 bin/tp3omp 16 10000 10000
ladrun -np 4 bin/tp3omp 32 10000 10000

ladrun -np 8 bin/tp3omp 1 10000 10000
ladrun -np 8 bin/tp3omp 2 10000 10000
ladrun -np 8 bin/tp3omp 4 10000 10000
ladrun -np 8 bin/tp3omp 8 10000 10000
ladrun -np 8 bin/tp3omp 16 10000 10000
ladrun -np 8 bin/tp3omp 32 10000 10000

