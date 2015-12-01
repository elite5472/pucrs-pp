default:
	mkdir -p bin && ladcomp -env mpiCC -fopenmp src/tp3omp.cpp -o bin/tp3omp
